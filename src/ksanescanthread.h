/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-11-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_SCAN_THREAD_H
#define KSANE_SCAN_THREAD_H

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QThread>
#include <QByteArray>

#define SCAN_READ_CHUNK_SIZE 100000

namespace KSaneIface
{
class KSaneScanThread: public QThread
{
    Q_OBJECT
public:
    typedef enum {
        READ_ON_GOING,
        READ_ERROR,
        READ_CANCEL,
        READ_READY
    } ReadStatus;

    KSaneScanThread(SANE_Handle handle, QByteArray *data);
    void run() override;
    void setImageInverted(bool);
    void cancelScan();
    int scanProgress();
    bool saneStartDone();

    ReadStatus frameStatus();
    SANE_Status saneStatus();
    SANE_Parameters saneParameters();

private:
    void readData();
    void copyToScanData(int readBytes);

    SANE_Byte       m_readData[SCAN_READ_CHUNK_SIZE];
    QByteArray     *m_data;
    SANE_Handle     m_saneHandle;
    int             m_frameSize;
    int             m_frameRead;
    int             m_frame_t_count;
    int             m_dataSize;
    SANE_Parameters m_params;
    SANE_Status     m_saneStatus;
    ReadStatus      m_readStatus;
    bool            m_saneStartDone;
    bool            m_invertColors;
};
}

#endif
