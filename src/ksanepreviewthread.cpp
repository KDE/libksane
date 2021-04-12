/* ============================================================
*
* SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
* SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#include "ksanepreviewthread.h"

#include <QMutexLocker>
#include <QImage>

#include <ksane_debug.h>

namespace KSaneIface
{
KSanePreviewThread::KSanePreviewThread(SANE_Handle handle, QImage *img):
    QThread(),
    m_saneHandle(handle),
    m_frameSize(0),
    m_frameRead(0),
    m_frame_t_count(0),
    m_dataSize(0),
    m_saneStatus(SANE_STATUS_GOOD),
    m_readStatus(READ_READY),
//    m_scanProgress(0),
    m_saneStartDone(false),
    m_invertColors(false),
    m_imageBuilder(img)
{
}

void KSanePreviewThread::setPreviewInverted(bool inverted)
{
    m_invertColors = inverted;
}

SANE_Status KSanePreviewThread::saneStatus()
{
    return m_saneStatus;
}

void KSanePreviewThread::cancelScan()
{
    m_readStatus = READ_CANCEL;
}

void KSanePreviewThread::run()
{
    m_dataSize = 0;
    m_readStatus = READ_ON_GOING;
    m_saneStartDone = false;

    // Start the scanning with sane_start
    m_saneStatus = sane_start(m_saneHandle);

    if (m_saneStatus != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_start=" << sane_strstatus(m_saneStatus);
        sane_cancel(m_saneHandle);
        m_readStatus = READ_ERROR;
        return;
    }

    // Read image parameters
    m_saneStatus = sane_get_parameters(m_saneHandle, &m_params);
    if (m_saneStatus != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_get_parameters=" << sane_strstatus(m_saneStatus);
        sane_cancel(m_saneHandle);
        m_readStatus = READ_ERROR;
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

    // set the m_saneStartDone here so the new QImage gets allocated before updating the preview.
    m_saneStartDone = true;

    while (m_readStatus == READ_ON_GOING) {
        readData();
    }
}

int KSanePreviewThread::scanProgress()
{
    // handscanners have negative data size
    if (m_dataSize <= 0) {
        return 0;
    }

    int bytesRead;

    if (m_frameSize < m_dataSize) {
        bytesRead = m_frameRead + (m_frameSize * m_frame_t_count);
    } else {
        bytesRead = m_frameRead;
    }

    return (int)(((float)bytesRead * 100.0) / m_dataSize);
}

void KSanePreviewThread::readData()
{
    SANE_Int readBytes;
    m_saneStatus = sane_read(m_saneHandle, m_readData, PREVIEW_READ_CHUNK_SIZE, &readBytes);

    switch (m_saneStatus) {
    case SANE_STATUS_GOOD:
        // continue to parsing the data
        break;

    case SANE_STATUS_EOF:
        // (handscanners have negative frame size)
        if (m_frameRead < m_frameSize) {
            qCDebug(KSANE_LOG) << "frameRead =" << m_frameRead  << ", frameSize =" << m_frameSize;
            m_readStatus = READ_ERROR;
            return;
        }
        if (m_params.last_frame == SANE_TRUE) {
            // this is where it all ends well :)
            m_readStatus = READ_READY;
            return;
        } else {
            // start reading next frame
            SANE_Status status = sane_start(m_saneHandle);
            if (status != SANE_STATUS_GOOD) {
                qCDebug(KSANE_LOG) << "sane_start =" << sane_strstatus(status);
                m_readStatus = READ_ERROR;
                return;
            }
            status = sane_get_parameters(m_saneHandle, &m_params);
            if (status != SANE_STATUS_GOOD) {
                qCDebug(KSANE_LOG) << "sane_get_parameters =" << sane_strstatus(status);
                m_readStatus = READ_ERROR;
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
        m_readStatus = READ_ERROR;
        sane_cancel(m_saneHandle);
        return;
    }

    copyToPreviewImg(readBytes);
}

void KSanePreviewThread::copyToPreviewImg(int readBytes)
{
    QMutexLocker locker(&imgMutex);
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

    if (m_imageBuilder.copyToImage(m_readData, readBytes)) {
        m_frameRead += readBytes;
    } else {
        m_readStatus = READ_ERROR;
    }
}

bool KSanePreviewThread::saneStartDone()
{
    return   m_saneStartDone;
}

bool KSanePreviewThread::imageResized()
{
    return m_imageBuilder.imageResized();
}

}  // NameSpace KSaneIface
