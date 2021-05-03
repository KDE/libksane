/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_FIND_DEVICES_THREAD_H
#define KSANE_FIND_DEVICES_THREAD_H

#include "ksanecore.h"

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

    const QList<KSaneCore::DeviceInfo> devicesList() const;

private:
    FindSaneDevicesThread();

    QList<KSaneCore::DeviceInfo> m_deviceList;
};

}

#endif
