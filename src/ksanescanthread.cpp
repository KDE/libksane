/* ============================================================
*
* SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
* SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
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
KSaneScanThread::KSaneScanThread(SANE_Handle handle, QImage *img, QByteArray *data):
    QThread(), m_saneHandle(handle), m_imageBuilder(img), m_data(data), m_image(img)
{
    m_emitProgressUpdateTimer.setSingleShot(false);
    m_emitProgressUpdateTimer.setInterval(300);
    connect(&m_emitProgressUpdateTimer, &QTimer::timeout, this, &KSaneScanThread::updateScanProgress);
    connect(this, &QThread::started, &m_emitProgressUpdateTimer, QOverload<>::of(&QTimer::start));
    connect(this, &QThread::finished, this, &KSaneScanThread::finishProgress);
}

void KSaneScanThread::setImageInverted(QVariant newValue)
{
    const bool newInvert = newValue.toBool();
    if (m_invertColors != newInvert) {
        m_invertColors = newInvert;
        if (m_image != nullptr) {
            m_image->invertPixels();
        }
    }
}

void KSaneScanThread::setPreview(bool isPreview)
{
    m_isPreview = isPreview;
}

KSaneScanThread::ReadStatus KSaneScanThread::frameStatus()
{
    return m_readStatus;
}

SANE_Status KSaneScanThread::saneStatus()
{
    return m_saneStatus;
}

SANE_Parameters KSaneScanThread::saneParameters()
{
    return m_params;
}

void KSaneScanThread::lockImage()
{
    m_imageMutex.lock();
}

void KSaneScanThread::unlockImage()
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
    m_saneStartDone = false;

    // Start the scanning with sane_start
    m_saneStatus = sane_start(m_saneHandle);

    m_saneStartDone = true;

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

    if (m_isPreview) {
        m_imageBuilder.start(m_params);
    }
    m_frameRead = 0;
    m_frame_t_count = 0;
    
    m_data->clear();
    if (m_dataSize > 0) {
        m_data->reserve(m_dataSize);
    }

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

    Q_EMIT scanProgressUpdated(static_cast<int>((static_cast<float>(bytesRead) * 100.0) / m_dataSize));
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
    
    if (m_isPreview) {
        copyToPreviewImg(readBytes);
    } else {
        copyToByteArray(readBytes);  
    } 
}

void KSaneScanThread::copyToPreviewImg(int readBytes)
{  
    QMutexLocker locker(&m_imageMutex);
    if (m_imageBuilder.copyToImage(m_readData, readBytes)) {
        m_frameRead += readBytes;
    } else {
        m_readStatus = ReadError;
    }
}

#define index_red8_to_rgb8(i)     (i*3)
#define index_red16_to_rgb16(i)   ((i/2)*6 + i%2)

#define index_green8_to_rgb8(i)   (i*3 + 1)
#define index_green16_to_rgb16(i) ((i/2)*6 + i%2 + 2)

#define index_blue8_to_rgb8(i)    (i*3 + 2)
#define index_blue16_to_rgb16(i)  ((i/2)*6 + i%2 + 4)

void KSaneScanThread::copyToByteArray(int readBytes)
{
    switch (m_params.format) {
    case SANE_FRAME_GRAY:
        m_data->append(reinterpret_cast<const char *>(m_readData), readBytes);
        m_frameRead += readBytes;
        return;
    case SANE_FRAME_RGB:
        if (m_params.depth == 1) {
            break;
        }
        m_data->append(reinterpret_cast<const char *>(m_readData), readBytes);
        m_frameRead += readBytes;
        return;

    case SANE_FRAME_RED:
        if (m_params.depth == 8) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_red8_to_rgb8(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_red16_to_rgb16(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        }
        break;

    case SANE_FRAME_GREEN:
        if (m_params.depth == 8) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_green8_to_rgb8(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_green16_to_rgb16(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        }
        break;

    case SANE_FRAME_BLUE:
        if (m_params.depth == 8) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_blue8_to_rgb8(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < readBytes; i++) {
                (*m_data)[index_blue16_to_rgb16(m_frameRead)] = m_readData[i];
                m_frameRead++;
            }
            return;
        }
        break;
    }

    qCDebug(KSANE_LOG) << "Format" << m_params.format
             << "and depth" << m_params.depth
             << "is not yet supported by libksane!";
    m_readStatus = ReadError;
    return;
}

bool KSaneScanThread::saneStartDone()
{
    return m_saneStartDone;
}

bool KSaneScanThread::imageResized()
{
    return m_imageBuilder.imageResized();
}

}  // NameSpace KSaneIface
