/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2012-05-24
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2012 by Kare Sars <kare.sars@iki.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

#include "KSaneDevicePrivate.h"
#include "KSaneDevicePrivate.moc"

#include "KSaneFindDevicesThread.h"

#include <KDebug>

KSaneDevicePrivate::KSaneDevicePrivate(KSaneDevice *parent): q(parent) {}

void KSaneDevicePrivate::setDefaultValues()
{
    KSaneOption *opt;

    // Try to get Color mode by default
    if ((opt = option(SANE_NAME_SCAN_MODE)) != 0) {
        opt->setStrValue(i18n(SANE_VALUE_SCAN_MODE_COLOR));
    }

    // Try to set 8 bit color
    if ((opt = option(SANE_NAME_BIT_DEPTH)) != 0) {
        opt->setValue(8);
    }

    // Try to set Scan resolution to 600 DPI
    if ((opt = option(SANE_NAME_SCAN_RESOLUTION)) != 0) {
        opt->setValue(300);
    }
}

qreal KSaneDevicePrivate::currentXDPI()
{
    KSaneOption *dpi = option(SANE_NAME_SCAN_X_RESOLUTION);
    if (!dpi) {
        dpi = option(SANE_NAME_SCAN_RESOLUTION);
    }
    if (!dpi) {
        // failed to get the object
        return 0;
    }
    return dpi->maxValue();
}

qreal KSaneDevicePrivate::currentYDPI()
{
    KSaneOption *dpi = option(SANE_NAME_SCAN_Y_RESOLUTION);
    if (!dpi) {
        dpi = option(SANE_NAME_SCAN_RESOLUTION);
    }
    if (!dpi) {
        // failed to get the object
        return 0;
    }
    return dpi->maxValue();
}

KSaneOption *KSaneDevicePrivate::option(const QString &name)
{
    int i;
    for (i=0; i<m_optList.size(); i++) {
        if (m_optList.at(i)->saneName() == name) {
            return m_optList.at(i);
        }
    }
    return 0;
}

void KSaneDevicePrivate::updateProgress()
{
    if (m_isPreview) {
        emit (q->previewProgress(m_previewThread->scanProgress()));
    }
    else {
        emit (q->scanProgress(m_scanThread->scanProgress()));
    }
}

void KSaneDevicePrivate::scheduleValReload()
{
    m_readValsTmr.start(5);
}

void KSaneDevicePrivate::optReload()
{
    for (int i=0; i<m_optList.size(); i++) {
        m_optList.at(i)->readOption();
        // Also read the values
        m_optList.at(i)->readValue();
    }

    // estimate the preview size and create an empty image
    // this is done so that you can select scan area without
    // having to scan a preview.
    updatePreviewSize();
}

void KSaneDevicePrivate::valReload()
{
    for (int i=0; i<m_optList.size(); i++) {
        m_optList.at(i)->readValue();
    }
}


void KSaneDevicePrivate::updatePreviewSize()
{
    SANE_Parameters params;
    SANE_Status status;
    //check what image size we would get in a scan
    status = sane_get_parameters(m_saneHandle, &params);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "sane_get_parameters=" << sane_strstatus(status);
    }
    else {
        QSize newSize(params.pixels_per_line, params.lines);
        if ((newSize.width() != 0) &&
            (newSize.height() != 0) &&
            (newSize != m_previewThread->image()->size()))
        {
            m_previewThread->setPreviewImageSize(newSize);
        }
    }
}

void KSaneDevicePrivate::previewScanDone()
{
    m_updProgressTmr.stop();
    updateProgress();
    // even if the scan is finished successfully we need to call sane_cancel()
    sane_cancel(m_saneHandle);

    // FIXME check this
    if (m_closeDevicePending) {
        sane_close(m_saneHandle);
        m_saneHandle = 0;
        emit (q->previewDone());
        return;
    }

    // restore the original settings of the changed parameters
    KSaneOption *depth = option(SANE_NAME_BIT_DEPTH);
    KSaneOption *res = option(SANE_NAME_SCAN_RESOLUTION);
    KSaneOption *resX = option(SANE_NAME_SCAN_X_RESOLUTION);
    KSaneOption *resY = option(SANE_NAME_SCAN_Y_RESOLUTION);
    KSaneOption *preview = option(SANE_NAME_PREVIEW);

    if (depth != 0)   depth->restoreSavedData();
    if (res != 0)     res->restoreSavedData();
    if (resX != 0)    resX->restoreSavedData();
    if (resY != 0)    resY->restoreSavedData();
    if (preview != 0) preview->restoreSavedData();

    if ((m_previewThread->status != SANE_STATUS_GOOD) &&
        (m_previewThread->status != SANE_STATUS_EOF))
    {
        emit (q->userMessage(KSaneDevice::ErrorGeneral, i18n(sane_strstatus(m_previewThread->status))));
    }


    emit (q->previewDone());

    return;
}

KSaneDevice::ImageFormat KSaneDevicePrivate::imageFormat(SANE_Parameters &params)
{
    switch(params.format)
    {
        case SANE_FRAME_GRAY:
            switch(params.depth)
            {
                case 1:
                    return KSaneDevice::FormatBlackWhite;
                case 8:
                    return KSaneDevice::FormatGrayScale8;
                case 16:
                    return KSaneDevice::FormatGrayScale16;
                default:
                    return KSaneDevice::FormatNone;
            }
        case SANE_FRAME_RGB:
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
            switch (params.depth)
            {
                case 8:
                    return KSaneDevice::FormatRGB_8_C;
                case 16:
                    return KSaneDevice::FormatRGB_16_C;
                default:
                    return KSaneDevice::FormatNone;
            }
    }
    return KSaneDevice::FormatNone;
}

int KSaneDevicePrivate::bytesPerLines(SANE_Parameters &params)
{
    switch (imageFormat(params))
    {
        case KSaneDevice::FormatBlackWhite:
        case KSaneDevice::FormatGrayScale8:
        case KSaneDevice::FormatGrayScale16:
            return params.bytes_per_line;

        case KSaneDevice::FormatRGB_8_C:
            return params.pixels_per_line*3;

        case KSaneDevice::FormatRGB_16_C:
            return params.pixels_per_line*6;

        case KSaneDevice::FormatNone:
        case KSaneDevice::FormatBMP: // to remove warning (BMP is omly valid in the twain wrapper)
            return 0;
    }
    return 0;
}


void KSaneDevicePrivate::oneFinalScanDone()
{
    m_updProgressTmr.stop();
    updateProgress();

    // FIXME check this
    if (m_closeDevicePending) {
        sane_close(m_saneHandle);
        m_saneHandle = 0;
        return;
    }

    if (m_scanThread->frameStatus() == KSaneScanThread::READ_READY) {
        // scan finished OK
        SANE_Parameters params = m_scanThread->saneParameters();
        int lines = params.lines;
        if (lines <= 0) {
            // this is probably a handscanner -> calculate the size from the read data
            int bytesPerLine = qMax(bytesPerLines(params), 1); // ensure no div by 0
            lines = m_scanData.size() / bytesPerLine;
        }
        emit (q->imageReady(m_scanData,
                            params.pixels_per_line,
                            lines,
                            bytesPerLines(params),
                            imageFormat(params),
                            currentXDPI(),
                            currentYDPI())
             );

        // now check if we should have automatic ADF batch scaning
        KSaneOption *source = option(SANE_NAME_SCAN_SOURCE);
        if (source){
            if (source->strValue().contains("Automatic Document Feeder")) {
                // in batch mode only one area can be scanned per page
                m_updProgressTmr.start();
                m_scanThread->start();
                return;
            }
        }

        // Check if we have a "wait for button" batch scanning
        KSaneOption *waitForButton = option("wait-for-button");
        if (waitForButton && (waitForButton->strValue() == "true")) {
            // in batch mode only one area can be scanned per page
            m_updProgressTmr.start();
            m_scanThread->start();
            return;
        }

        // not batch scan, call sane_cancel to be able to change parameters.
        sane_cancel(m_saneHandle);
        emit (q->scanDone());
    }
    else {
        switch(m_scanThread->saneStatus())
        {
            case SANE_STATUS_GOOD:
            case SANE_STATUS_CANCELLED:
            case SANE_STATUS_EOF:
                break;
            case SANE_STATUS_NO_DOCS:
                emit (q->userMessage(KSaneDevice::Information, i18n(sane_strstatus(m_scanThread->saneStatus()))));
                emit (q->scanDone());
                break;
            case SANE_STATUS_UNSUPPORTED:
            case SANE_STATUS_IO_ERROR:
            case SANE_STATUS_NO_MEM:
            case SANE_STATUS_INVAL:
            case SANE_STATUS_JAMMED:
            case SANE_STATUS_COVER_OPEN:
            case SANE_STATUS_DEVICE_BUSY:
            case SANE_STATUS_ACCESS_DENIED:
                emit (q->userMessage(KSaneDevice::ErrorGeneral, i18n(sane_strstatus(m_scanThread->saneStatus()))));
                emit (q->scanDone());
                break;
        }
    }

    sane_cancel(m_saneHandle);
    m_scanOngoing = false;
}

