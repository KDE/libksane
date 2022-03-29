/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_SCAN_THREAD_H
#define KSANE_SCAN_THREAD_H

#include "ksaneimagebuilder.h"

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QThread>
#include <QMutex>
#include <QByteArray>
#include <QImage>
#include <QTimer>

#define SCAN_READ_CHUNK_SIZE 100000

namespace KSaneIface
{
class KSaneScanThread: public QThread
{
    Q_OBJECT
public:
    enum ReadStatus {
        ReadOngoing,
        ReadError,
        ReadCancel,
        ReadReady
    };

    KSaneScanThread(SANE_Handle handle);
    void run() override;
    void setImageInverted(const QVariant &newValue);
    void setImageResolution(const QVariant &newValue);
    void cancelScan();

    ReadStatus frameStatus();
    SANE_Status saneStatus();

    void lockScanImage();
    QImage *scanImage();
    void unlockScanImage();

Q_SIGNALS:

    void scanProgressUpdated(int progress);

private:
    void readData();
    void updateScanProgress();
    void copyToScanData(int readBytes);

    SANE_Byte       m_readData[SCAN_READ_CHUNK_SIZE];
    SANE_Handle     m_saneHandle;
    int             m_frameSize = 0;
    int             m_frameRead = 0;
    int             m_frame_t_count = 0;
    int             m_dataSize = 0;
    int             m_dpi = 0;
    SANE_Parameters m_params;
    SANE_Status     m_saneStatus = SANE_STATUS_GOOD;
    ReadStatus      m_readStatus = ReadReady;
    bool            m_announceFirstRead = true;
    bool            m_invertColors = false;
    KSaneImageBuilder m_imageBuilder;
    QImage          m_image;
    QMutex          m_imageMutex;

    QTimer          m_emitProgressUpdateTimer;
};
}

#endif // KSANE_SCAN_THREAD_H
