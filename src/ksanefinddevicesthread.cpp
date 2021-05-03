/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanefinddevicesthread.h"

// #include "ksanewidget_p.h"

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QMutex>
#include <QMutexLocker>

namespace KSaneIface
{
static FindSaneDevicesThread *s_instancesane = nullptr;
Q_GLOBAL_STATIC(QMutex, s_mutexsane)

FindSaneDevicesThread *FindSaneDevicesThread::getInstance()
{
    QMutexLocker locker(s_mutexsane);

    if (s_instancesane == nullptr) {
        s_instancesane = new FindSaneDevicesThread();
    }

    return s_instancesane;
}

FindSaneDevicesThread::FindSaneDevicesThread() : QThread(nullptr)
{
}

FindSaneDevicesThread::~FindSaneDevicesThread()
{
    QMutexLocker locker(s_mutexsane);
    wait();
}

void FindSaneDevicesThread::run()
{
    SANE_Device const **devList;
    //SANE_Int            version;
    SANE_Status         status;

    // This is unfortunately not very reliable as many back-ends do not refresh
    // the device list after the sane_init() call...
    status = sane_get_devices(&devList, SANE_FALSE);

    m_deviceList.clear();
    if (status == SANE_STATUS_GOOD) {
        int i = 0;
        KSaneCore::DeviceInfo deviceInfo;

        while (devList[i] != nullptr) {
            deviceInfo.name = QString::fromUtf8(devList[i]->name);
            deviceInfo.vendor = QString::fromUtf8(devList[i]->vendor);
            deviceInfo.model = QString::fromUtf8(devList[i]->model);
            deviceInfo.type = QString::fromUtf8(devList[i]->type);
            m_deviceList << deviceInfo;
            i++;
        }
    }
}

const QList<KSaneCore::DeviceInfo> FindSaneDevicesThread::devicesList() const
{
    return m_deviceList;
}

}
