/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2011-12-20
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2011 by Kare Sars <kare.sars@iki.fi>
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

#include "KSaneDevice.h"
#include "KSaneDevice.moc"

#include "KSaneDevicePrivate.h"

#include "KSaneFindDevicesThread.h"
#include "KSaneAccessSingleton.h"

#include "KSaneOption.h"
#include "KSaneOptButton.h"
#include "KSaneOptCheckBox.h"
#include "KSaneOptCombo.h"
#include "KSaneOptEntry.h"
#include "KSaneOptGamma.h"
#include "KSaneOptSlider.h"
#include "KSaneOptSliderF.h"

#include <KDebug>
#include <QVarLengthArray>
#include <QMutexLocker>

KSaneDevice::KSaneDevice(QObject *parent): QObject(parent), d(new KSaneDevicePrivate(this))
{
    KSaneAccessSingleton *access = KSaneAccessSingleton::getInstance();
    access->reserveSaneInterface(this);

    KSaneFindDevicesThread *findDevThread = KSaneFindDevicesThread::getInstance();
    connect(findDevThread, SIGNAL(devicesListUpdated(QList<KSaneDevice::Info>)),
            this,          SIGNAL(availableDevices(QList<KSaneDevice::Info>)));

    connect(&d->m_updProgressTmr, SIGNAL(timeout()), d, SLOT(updateProgress()));
}

KSaneDevice::~KSaneDevice()
{
    KSaneAccessSingleton::getInstance()->releaseSaneInterface(this);
    delete d;
}

void KSaneDevice::requestDeviceList(bool rescanDeviceList)
{
    KSaneFindDevicesThread *findDevThread = KSaneFindDevicesThread::getInstance();
    if (rescanDeviceList) {
        findDevThread->start();
    }
    else if (findDevThread->devicesList().isEmpty()) {
        findDevThread->start();
    }
    else {
        emit availableDevices(findDevThread->devicesList());
    }
}

bool KSaneDevice::openDevice(const QString &name)
{
    const SANE_Option_Descriptor  *optDesc;
    SANE_Status                    status;
    SANE_Word                      numSaneOptions;
    SANE_Int                       res;

    // don't bother trying to open if the device string is empty
    if (name.isEmpty()) {
        return false;
    }
    // save the device name
    d->m_info.name = name;

    // Try to open the device
    status = sane_open(name.toLatin1(), &d->m_saneHandle);

    if(status == SANE_STATUS_ACCESS_DENIED) {
        // FIXME
        qDebug() << "sane_open: SANE_STATUS_ACCESS_DENIED";
        d->m_info.name.clear();
        return false;
    }

    if (status != SANE_STATUS_GOOD) {
        kDebug() << name << ": sane_open() failed! status = " << sane_strstatus(status);
        d->m_info.name.clear();
        return false;
    }

    requestDeviceList(false);

    // Read the options (start with option 0 the number of parameters)
    optDesc = sane_get_option_descriptor(d->m_saneHandle, 0);
    if (optDesc == 0) {
        //d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_info.name.clear();
        return false;
    }

    QVarLengthArray<char> data(optDesc->size);
    status = sane_control_option(d->m_saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        //d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_info.name.clear();
        return false;
    }

    numSaneOptions = *reinterpret_cast<SANE_Word*>(data.data());

    // read the rest of the options
    for (int i=1; i<numSaneOptions; ++i) {
        switch (KSaneOption::optionType(sane_get_option_descriptor(d->m_saneHandle, i))) {
            case KSaneOption::TYPE_DETECT_FAIL:
                d->m_optList.append(new KSaneOption(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_CHECKBOX:
                d->m_optList.append(new KSaneOptCheckBox(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_SLIDER:
                d->m_optList.append(new KSaneOptSlider(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_SLIDER_F:
                d->m_optList.append(new KSaneOptSliderF(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_COMBO:
                d->m_optList.append(new KSaneOptCombo(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_ENTRY:
                d->m_optList.append(new KSaneOptEntry(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_GAMMA:
                d->m_optList.append(new KSaneOptGamma(d->m_saneHandle, i));
                break;
            case KSaneOption::TYPE_BUTTON:
                d->m_optList.append(new KSaneOptButton(d->m_saneHandle, i));
                break;
        }
    }

    // do the connections of the option parameters
    for (int i=1; i<d->m_optList.size(); ++i) {
        //kDebug() << d->m_optList.at(i)->saneName();
        connect (d->m_optList.at(i), SIGNAL(optsNeedReload()), d, SLOT(optReload()));
        connect (d->m_optList.at(i), SIGNAL(valsNeedReload()), d, SLOT(scheduleValReload()));

        if (d->m_optList.at(i)->needsPolling()) {
            //kDebug() << d->m_optList.at(i)->saneName() << " needs polling";
            d->m_pollList.append(d->m_optList.at(i));
            KSaneOptCheckBox *buttonOption = qobject_cast<KSaneOptCheckBox *>(d->m_optList.at(i));
            if (buttonOption) {
                connect(buttonOption, SIGNAL(buttonPressed(QString,QString,bool)),
                        this, SIGNAL(buttonPressed(QString,QString,bool)));
            }
        }
    }

    // start polling the poll options
    if (d->m_pollList.size() > 0) {
        d->m_optionPollTmr.start();
    }

    // Create the preview thread
    d->m_previewThread = new KSanePreviewThread(d->m_saneHandle);
    connect(d->m_previewThread, SIGNAL(finished()), d, SLOT(previewScanDone()));
    connect(d->m_previewThread, SIGNAL(imageResized()), this, SIGNAL(previewImageResized()));

    // Create the read thread
    d->m_scanThread = new KSaneScanThread(d->m_saneHandle, &d->m_scanData);
    connect(d->m_scanThread, SIGNAL(finished()), d, SLOT(oneFinalScanDone()));

    // try to set KSaneWidget default values
    d->setDefaultValues();

    return true;
}

#define incPixel(x,y,ppl) { x++; if (x>=ppl) { y++; x=0;} }

QImage KSaneDevice::toQImage(const QByteArray &data,
                             int width, int height, int bytes_per_line,
                             ImageFormat format, qreal xdpi, qreal ydpi)
{
    QImage img;
    int j=0;
    int pixel_x = 0;
    int pixel_y = 0;

    switch (format)
    {
        case FormatBlackWhite:
        {
            QVector<QRgb> table;
            img = QImage((uchar*)data.data(), width,  height,
                         bytes_per_line, QImage::Format_Mono);
            // The color table must be set
            table.append(0xFFFFFFFF);
            table.append(0xFF000000);
            img.setColorTable(table);
            break;
        }
        case FormatGrayScale8:
            img = QImage(width, height, QImage::Format_RGB32);
            j=0;
            for (int i=0; i<data.size(); i++) {
                img.bits()[j+0] = data.data()[i];
                img.bits()[j+1] = data.data()[i];
                img.bits()[j+2] = data.data()[i];
                j+=4;
            }
            break;

        case FormatGrayScale16:
            img = QImage(width, height, QImage::Format_RGB32);
            j=0;
            for (int i=1; i<data.size(); i+=2) {
                img.bits()[j+0] = data.data()[i];
                img.bits()[j+1] = data.data()[i];
                img.bits()[j+2] = data.data()[i];
                j+=4;
            }
            break;

        case FormatRGB_8_C:
            pixel_x = 0;
            pixel_y = 0;

            img = QImage(width, height, QImage::Format_RGB32);

            for (int i=0; i<data.size(); i+=3) {
                img.setPixel(pixel_x, pixel_y,
                             qRgb(data[i], data[i+1], data[i+2]));
                incPixel(pixel_x, pixel_y, width);
            }
            break;

        case FormatRGB_16_C:
            pixel_x = 0;
            pixel_y = 0;

            img = QImage(width, height, QImage::Format_RGB32);

            for (int i=1; i<data.size(); i+=6) {
                img.setPixel(pixel_x, pixel_y,
                             qRgb(data[i], data[i+2], data[i+4]));
                incPixel(pixel_x, pixel_y, width);
            }
            break;

        case FormatNone:
        default:
            kDebug() << "Unsupported conversion";
            break;
    }
    img.setDotsPerMeterX(xdpi * (1000.0 / 25.4));
    img.setDotsPerMeterY(ydpi * (1000.0 / 25.4));
    return img;
}

const KSaneDevice::Info KSaneDevice::info() const
{
    return d->m_info;
}

qreal KSaneDevice::scanAreaWidth()
{
    qreal currentDPI = d->currentXDPI();
    if (currentDPI == 0) {
        return 0;
    }

    KSaneOption *brx = d->option(SANE_NAME_SCAN_BR_X);

    if (brx) {
        if (brx->unit() == SANE_UNIT_PIXEL) {
            return brx->maxValue() / currentDPI / 25.4; // convert dpi to dpmm
        }
        else if (brx->unit() == SANE_UNIT_MM) {
            return brx->maxValue();
        }
    }
    return 0;
}

qreal KSaneDevice::scanAreaHeight()
{
    qreal currentDPI = d->currentYDPI();
    if (currentDPI == 0) {
        // failed to read the value
        return 0;
    }

    KSaneOption *bry = d->option(SANE_NAME_SCAN_BR_Y);

    if (bry) {
        if (bry->unit() == SANE_UNIT_PIXEL) {
            return bry->maxValue() / currentDPI / 25.4; // convert dpi to dpmm
        }
        else if (bry->unit() == SANE_UNIT_MM) {
            return bry->maxValue();
        }
    }
    return 0;
}

bool KSaneDevice::setSelection(QPointF topLeft, QPointF bottomRight)
{
    KSaneOption *tlx = d->option(SANE_NAME_SCAN_TL_X);
    KSaneOption *tly = d->option(SANE_NAME_SCAN_TL_Y);
    KSaneOption *brx = d->option(SANE_NAME_SCAN_BR_X);
    KSaneOption *bry = d->option(SANE_NAME_SCAN_BR_Y);
    
    if (!brx || !bry || !tlx || !tly) {
        return false;
    }
    qreal xmax = brx->maxValue();
    qreal ymax = bry->maxValue();

    if (topLeft.x() < 0.0 || topLeft.y() < 0.0 || bottomRight.x() < 0.0 || bottomRight.y() < 0.0) {
        tlx->setValue(0.0);
        tly->setValue(0.0);
        brx->setValue(xmax);
        bry->setValue(ymax);
    }
    
    if (bry->unit() == SANE_UNIT_MM) {
        // clear selection if values are out of bounds
        if (topLeft.x() > xmax || topLeft.y() > ymax || bottomRight.x() > xmax || bottomRight.y() > ymax) {
            tlx->setValue(0.0);
            tly->setValue(0.0);
            brx->setValue(xmax);
            bry->setValue(ymax);
            return false;
        }
        else {
            tlx->setValue(topLeft.x());
            tly->setValue(topLeft.y());
            brx->setValue(bottomRight.x());
            bry->setValue(bottomRight.y());
            return true;
        }
    }
    else if (bry->unit() == SANE_UNIT_PIXEL) {
        const qreal mmperinch = 25.4;
        const qreal dpix = d->currentXDPI();
        const qreal dpiy = d->currentYDPI();
        const qreal mx = dpix / mmperinch;
        const qreal my = dpiy / mmperinch;

        if (mx*topLeft.x() > xmax ||
            my*topLeft.y() > ymax ||
            mx*bottomRight.x() > xmax ||
            my*bottomRight.y() > ymax)
        {
            tlx->setValue(0.0);
            tly->setValue(0.0);
            brx->setValue(xmax);
            bry->setValue(ymax);
            return false;
        }
        else {
            tlx->setValue(mx*topLeft.x());
            tly->setValue(my*topLeft.y());
            brx->setValue(mx*bottomRight.x());
            bry->setValue(my*bottomRight.y());
            return true;
        }
    }
    return false;
}

void KSaneDevice::setPreviewResolution(qreal dpi)
{
    d->m_previewDPI = dpi;
}

const QImage *KSaneDevice::previewImage()
{
    if (!d->m_previewThread) {
        return 0;
    }
    return d->m_previewThread->image();
}

QMutex *KSaneDevice::previewImageMutex()
{
    if (!d->m_previewThread) {
        return 0;
    }
    return d->m_previewThread->imageMutex();
}

void KSaneDevice::setColorsInverted(bool invert)
{
    QMutexLocker locker(d->m_previewThread->imageMutex());

    d->m_previewThread->setImageInverted(invert);
    emit (previewImageResized());
}

KSaneOption *KSaneDevice::option(const QString &name)
{
    return d->option(name);
}

KSaneOption *KSaneDevice::optionSource()    {return d->option(SANE_NAME_SCAN_SOURCE);}
KSaneOption *KSaneDevice::optionColorMode() {return d->option(SANE_NAME_SCAN_MODE);}
KSaneOption *KSaneDevice::optionColorDepth(){return d->option(SANE_NAME_BIT_DEPTH);}


const QList<KSaneOption*>KSaneDevice::options()
{
    return d->m_optList;
}

void KSaneDevice::cancelScan()
{
    if (d->m_scanThread->isRunning()) {
        d->m_scanThread->cancelScan();
    }
    if (d->m_previewThread->isRunning()) {
        d->m_previewThread->cancelScan();
    }
}

void KSaneDevice::scanPreview()
{
    if (d->m_scanOngoing) return;
    d->m_scanOngoing = true;
    d->m_isPreview = true;

    // store the current settings of parameters before changing them
    KSaneOption *depth   = d->option(SANE_NAME_BIT_DEPTH);
    KSaneOption *res     = d->option(SANE_NAME_SCAN_RESOLUTION);
    KSaneOption *resX    = d->option(SANE_NAME_SCAN_X_RESOLUTION);
    KSaneOption *resY    = d->option(SANE_NAME_SCAN_Y_RESOLUTION);
    KSaneOption *preview = d->option(SANE_NAME_PREVIEW);

    if (depth != 0)   depth->storeCurrentData();
    if (res != 0)     res->storeCurrentData();
    if (resX != 0)    resX->storeCurrentData();
    if (resY != 0)    resY->storeCurrentData();
    if (preview != 0) preview->storeCurrentData();

    // Select the whole page if possible
    KSaneOption *tlx = d->option(SANE_NAME_SCAN_TL_X);
    KSaneOption *tly = d->option(SANE_NAME_SCAN_TL_Y);
    KSaneOption *brx = d->option(SANE_NAME_SCAN_BR_X);
    KSaneOption *bry = d->option(SANE_NAME_SCAN_BR_Y);

    if ((tlx != 0) && (tly != 0) && (brx != 0) && (bry != 0)) {
        // get maximums
        qreal xmax = brx->maxValue();
        qreal ymax = bry->maxValue();

        // select the whole area
        tlx->setValue(0);
        tly->setValue(0);
        brx->setValue(xmax);
        bry->setValue(ymax);
    }

    if (res != 0) {
        if (d->m_previewDPI >= 25.0) {
            res->setValue(d->m_previewDPI);
        }
        else {
            // set the res-option to getMinValue and increase if necessary
            SANE_Parameters params;
            SANE_Status status;
            qreal dpi = res->minValue();
            do {
                res->setValue(dpi);
                // FIXME check resx, resy
                //check what image size we would get in a scan
                status = sane_get_parameters(d->m_saneHandle, &params);
                if (status != SANE_STATUS_GOOD) {
                    emit (userMessage(KSaneDevice::ErrorGeneral, i18n(sane_strstatus(status))));
                    emit (previewDone());
                    return;
                }

                if (dpi > 600) break;

                // Increase the dpi value
                dpi += 25.0;
            }
            while ((params.pixels_per_line < 300) || ((params.lines > 0) && (params.lines < 300)));

            if (params.pixels_per_line == 0) {
                // This is a security measure for broken backends
                res->setValue(res->minValue());
                kDebug() << "Setting minimum DPI value for a broken back-end";
            }
        }
    }

    // set preview option to true if possible
    if (preview != 0) preview->setValue(SANE_TRUE);


    // execute any pending value reload
    while (d->m_readValsTmr.isActive()) {
        d->m_readValsTmr.stop();
        d->valReload();
    }

    emit (previewProgress(0));
    d->m_updProgressTmr.start();
    d->m_previewThread->setImageInverted(d->m_invertColors);
    d->m_previewThread->start();
}

void KSaneDevice::scanFinal()
{
    if (d->m_scanOngoing) return;
    d->m_scanOngoing = true;
    d->m_isPreview = false;

    // execute any pending value reload
    while (d->m_readValsTmr.isActive()) {
        d->m_readValsTmr.stop();
        d->valReload();
    }

    emit scanProgress(0);
    d->m_updProgressTmr.start();
    d->m_scanThread->setImageInverted(d->m_invertColors);
    d->m_scanThread->start();
}


