/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2009-11-13
* Description : Sane interface for KDE
*
* Copyright (C) 2009 by Kare Sars <kare dot sars at iki dot fi>
* Copyright (C) 2014 by Gregor Mitsch: port to KDE5 frameworks
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

#include "ksanepreviewthread.h"

#include <QMutexLocker>
#include <QDebug>
#include <QImage>

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
        qDebug() << "sane_start=" << sane_strstatus(m_saneStatus);
        sane_cancel(m_saneHandle);
        m_readStatus = READ_ERROR;
        return;
    }

    // Read image parameters
    m_saneStatus = sane_get_parameters(m_saneHandle, &m_params);
    if (m_saneStatus != SANE_STATUS_GOOD) {
        qDebug() << "sane_get_parameters=" << sane_strstatus(m_saneStatus);
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
            qDebug() << "frameRead =" << m_frameRead  << ", frameSize =" << m_frameSize;
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
                qDebug() << "sane_start =" << sane_strstatus(status);
                m_readStatus = READ_ERROR;
                return;
            }
            status = sane_get_parameters(m_saneHandle, &m_params);
            if (status != SANE_STATUS_GOOD) {
                qDebug() << "sane_get_parameters =" << sane_strstatus(status);
                m_readStatus = READ_ERROR;
                sane_cancel(m_saneHandle);
                return;
            }
            //qDebug() << "New Frame";
            m_imageBuilder.beginFrame(m_params);
            m_frameRead = 0;
            m_frame_t_count++;
            break;
        }
    default:
        qDebug() << "sane_read=" << m_saneStatus << "=" << sane_strstatus(m_saneStatus);
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
            //if (readBytes%2) qDebug() << "readBytes=" << readBytes;
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
