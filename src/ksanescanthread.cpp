/* ============================================================
*
* SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
* SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
* SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#include "ksanescanthread.h"

#include <QMutexLocker>
#include <QImage>

#include <ksane_debug.h>

namespace KSaneIface
{
KSaneScanThread::KSaneScanThread(SANE_Handle handle):
    QThread(), m_saneHandle(handle), m_imageBuilder(&m_image, &m_dpi)
{
    m_emitProgressUpdateTimer.setSingleShot(false);
    m_emitProgressUpdateTimer.setInterval(300);
    connect(&m_emitProgressUpdateTimer, &QTimer::timeout, this, &KSaneScanThread::updateScanProgress);
    connect(this, &QThread::started, &m_emitProgressUpdateTimer, QOverload<>::of(&QTimer::start));
    connect(this, &QThread::finished, this, &KSaneScanThread::finishProgress);
}

void KSaneScanThread::setImageInverted(const QVariant &newValue)
{
    const bool newInvert = newValue.toBool();
    if (m_invertColors != newInvert) {
        m_invertColors = newInvert;
        m_image.invertPixels();
    }
}

void KSaneScanThread::setImageResolution(const QVariant &newValue)
{
    bool ok;
    const int newDPI = newValue.toInt(&ok);
    if (ok && m_dpi != newDPI) {
        m_dpi = newDPI;
    }
}

KSaneScanThread::ReadStatus KSaneScanThread::frameStatus()
{
    return m_readStatus;
}

SANE_Status KSaneScanThread::saneStatus()
{
    return m_saneStatus;
}

QImage *KSaneScanThread::scanImage()
{
    return &m_image;
}

void KSaneScanThread::lockScanImage()
{
    m_imageMutex.lock();
}

void KSaneScanThread::unlockScanImage()
{
    m_imageMutex.unlock();
}

void KSaneScanThread::cancelScan()
{
    m_readStatus = ReadCancel;
}

void KSaneScanThread::run()
{
    m_dataSize = 0;
    m_readStatus = ReadOngoing;

    // Start the scanning with sane_start
    m_saneStatus = sane_start(m_saneHandle);

    if (m_readStatus == ReadCancel) {
        return;
    }

    if (m_saneStatus != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_start=" << sane_strstatus(m_saneStatus);
        sane_cancel(m_saneHandle);
        m_readStatus = ReadError;
        return;
    }

    // Read image parameters
    m_saneStatus = sane_get_parameters(m_saneHandle, &m_params);
    if (m_saneStatus != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_get_parameters=" << sane_strstatus(m_saneStatus);
        sane_cancel(m_saneHandle);
        m_readStatus = ReadError;
        return;
    }

    // calculate data size
    m_frameSize  = m_params.lines * m_params.bytes_per_line;
    if ((m_params.format == SANE_FRAME_RED) ||
            (m_params.format == SANE_FRAME_GREEN) ||
            (m_params.format == SANE_FRAME_BLUE)) {
        // this is unfortunately calculated again for every frame....
        m_dataSize = m_frameSize * 3;
    } else {
        m_dataSize = m_frameSize;
    }

    m_imageBuilder.start(m_params);
    m_frameRead = 0;
    m_frame_t_count = 0;

    Q_EMIT scanProgressUpdated(0);

    while (m_readStatus == ReadOngoing) {
        readData();
    }
}

void KSaneScanThread::updateScanProgress()
{
    // handscanners have negative data size
    if (m_dataSize <= 0) {
        return;
    }

    int bytesRead;

    if (m_frameSize < m_dataSize) {
        bytesRead = m_frameRead + (m_frameSize * m_frame_t_count);
    } else {
        bytesRead = m_frameRead;
    }

    if (bytesRead > 0) {
        Q_EMIT scanProgressUpdated(static_cast<int>((static_cast<float>(bytesRead) * 100.0) / m_dataSize));
    }
}

void KSaneScanThread::finishProgress()
{
    m_emitProgressUpdateTimer.stop();
    Q_EMIT scanProgressUpdated(100);
}

void KSaneScanThread::readData()
{
    SANE_Int readBytes = 0;
    m_saneStatus = sane_read(m_saneHandle, m_readData, SCAN_READ_CHUNK_SIZE, &readBytes);

    switch (m_saneStatus) {
    case SANE_STATUS_GOOD:
        // continue to parsing the data
        break;

    case SANE_STATUS_EOF:
        // (handscanners have negative frame size)
        if (m_frameRead < m_frameSize) {
            qCDebug(KSANE_LOG) << "frameRead =" << m_frameRead  << ", frameSize =" << m_frameSize << "readBytes =" << readBytes;
            if ((readBytes > 0) && ((m_frameRead + readBytes) <= m_frameSize)) {
                qCDebug(KSANE_LOG) << "This is not a standard compliant backend";
                copyToScanData(readBytes);
            }
            // There are broken backends that return wrong number for bytes_per_line
            if (m_params.depth == 1 && m_params.lines > 0 && m_params.lines * m_params.pixels_per_line <= m_frameRead * 8) {
                qCDebug(KSANE_LOG) << "Warning!! This backend seems to return wrong bytes_per_line for line-art images!";
                qCDebug(KSANE_LOG) << "Warning!! Trying to correct the value!";
                m_params.bytes_per_line = m_frameRead / m_params.lines;
            }
            m_readStatus = ReadReady; // It is better to return a broken image than nothing
            return;
        }
        if (m_params.last_frame == SANE_TRUE) {
            // this is where it all ends well :)
            m_readStatus = ReadReady;
            return;
        } else {
            // start reading next frame
            m_saneStatus = sane_start(m_saneHandle);
            if (m_saneStatus != SANE_STATUS_GOOD) {
                qCDebug(KSANE_LOG) << "sane_start =" << sane_strstatus(m_saneStatus);
                m_readStatus = ReadError;
                return;
            }
            m_saneStatus = sane_get_parameters(m_saneHandle, &m_params);
            if (m_saneStatus != SANE_STATUS_GOOD) {
                qCDebug(KSANE_LOG) << "sane_get_parameters =" << sane_strstatus(m_saneStatus);
                m_readStatus = ReadError;
                sane_cancel(m_saneHandle);
                return;
            }
            //qCDebug(KSANE_LOG) << "New Frame";
            m_imageBuilder.beginFrame(m_params);
            m_frameRead = 0;
            m_frame_t_count++;
            break;
        }
    default:
        qCDebug(KSANE_LOG) << "sane_read=" << m_saneStatus << "=" << sane_strstatus(m_saneStatus);
        m_readStatus = ReadError;
        sane_cancel(m_saneHandle);
        return;
    }

    copyToScanData(readBytes);
}

void KSaneScanThread::copyToScanData(int readBytes)
{
    if (m_invertColors) {
        if (m_params.depth == 16) {
            //if (readBytes%2) qCDebug(KSANE_LOG) << "readBytes=" << readBytes;
            quint16 *u16ptr = reinterpret_cast<quint16 *>(m_readData);
            for (int i = 0; i < readBytes / 2; i++) {
                u16ptr[i] = 0xFFFF - u16ptr[i];
            }
        } else if (m_params.depth == 8) {
            for (int i = 0; i < readBytes; i++) {
                m_readData[i] = 0xFF - m_readData[i];
            }
        } else if (m_params.depth == 1) {
            for (int i = 0; i < readBytes; i++) {
                m_readData[i] = ~m_readData[i];
            }
        }
    }
    
    QMutexLocker locker(&m_imageMutex);
    if (m_imageBuilder.copyToImage(m_readData, readBytes)) {
        m_frameRead += readBytes;
    } else {
        m_readStatus = ReadError;
    }
}

}  // NameSpace KSaneIface
