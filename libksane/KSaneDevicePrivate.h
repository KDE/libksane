/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Copyright (C) 2012 by Kåre Särs <kare.sars@iki.fi>
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

#ifndef KSaneDevicePrivate_h
#define KSaneDevicePrivate_h

#include "KSaneDevice.h"
#include "KSaneOptInternal.h"
#include "KSaneScanThread.h"
#include "KSanePreviewThread.h"

#include <QObject>
#include <QTimer>

// Sane includes
extern "C"
{
    #include <sane/saneopts.h>
    #include <sane/sane.h>
}

class KSaneDevicePrivate: public QObject
{
    Q_OBJECT
public:
    KSaneDevicePrivate(KSaneDevice *parent);

    void setDefaultValues();
    KSaneOptInternal *optionInternal(const QString &name);
    KSaneOption *option(const QString &name);
    qreal currentXDPI();
    qreal currentYDPI();

    int bytesPerLines(SANE_Parameters &params);
    KSaneDevice::ImageFormat imageFormat(SANE_Parameters &params);

public Q_SLOTS:
    void scheduleValReload();
    void optReload();
    void valReload();

    void updatePreviewSize();

    void updateProgress();
    void previewScanDone();
    void oneFinalScanDone();

public:
    // variables
    KSaneDevice *q;

    // Device and parameter variables
    SANE_Handle        m_saneHandle;
    KSaneDevice::Info  m_info;
    bool               m_closeDevicePending;

    QList<KSaneOptInternal*> m_optIntList;
    QList<KSaneOptInternal*> m_pollList;
    QList<KSaneOption*> m_optList;
    QTimer              m_optionPollTmr;
    QTimer              m_readValsTmr;
    bool                m_invertColors;
    qreal               m_previewDPI;

    // scanning variables
    QTimer              m_updProgressTmr;
    bool                m_scanOngoing;
    bool                m_isPreview;
    KSanePreviewThread *m_previewThread;
    QImage             *m_previewImg;

    KSaneScanThread    *m_scanThread;
    QByteArray          m_scanData;

};


#endif
