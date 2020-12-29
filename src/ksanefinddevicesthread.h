/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_FIND_DEVICES_THREAD_H
#define KSANE_FIND_DEVICES_THREAD_H

#include "ksanewidget.h"

#include <QThread>
#include <QList>

namespace KSaneIface
{

class FindSaneDevicesThread : public QThread
{
    Q_OBJECT

public:
    static FindSaneDevicesThread *getInstance();
    ~FindSaneDevicesThread();
    void run() override;

    const QList<KSaneWidget::DeviceInfo> devicesList() const;

private:
    FindSaneDevicesThread();

    QList<KSaneWidget::DeviceInfo> m_deviceList;
};

}

#endif
