/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_PREVIEW_THREAD_H
#define KSANE_PREVIEW_THREAD_H

#include "ksanepreviewimagebuilder.h"

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QThread>
#include <QMutex>
#include <QImage>

#define PREVIEW_READ_CHUNK_SIZE 100000

namespace KSaneIface
{
class KSanePreviewThread: public QThread
{
    Q_OBJECT
public:
    typedef enum {
        READ_ON_GOING,
        READ_ERROR,
        READ_CANCEL,
        READ_READY
    } ReadStatus;

    KSanePreviewThread(SANE_Handle handle, QImage *img);
    void run() override;
    void setPreviewInverted(bool);
    void cancelScan();
    int scanProgress();
    bool saneStartDone();
    bool imageResized();

    SANE_Status saneStatus();

    QMutex imgMutex;

private:
    void readData();
    void copyToPreviewImg(int readBytes);

    SANE_Byte       m_readData[PREVIEW_READ_CHUNK_SIZE];
    SANE_Handle     m_saneHandle;
    int             m_frameSize;
    int             m_frameRead;
    int             m_frame_t_count;
    int             m_dataSize;
    SANE_Parameters m_params;
    SANE_Status     m_saneStatus;
    ReadStatus      m_readStatus;
//            int             m_scanProgress;
    bool            m_saneStartDone;
    bool            m_invertColors;
    KSanePreviewImageBuilder m_imageBuilder;
};
}

#endif
