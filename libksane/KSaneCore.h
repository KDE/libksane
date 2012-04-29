/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2011-12-28
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2011 by Kare Sars <kars.sars@iki.fi>
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

#ifndef KSANECORE_H
#define KSANECORE_H

#include <QObject>
#include "libksane_export.h"

class KSaneCorePrivate;

/**
 * This class provides the core provides the central access to sane.
 * @author Kare Sars <kare.sars@iki.fi>
 */
class LIBKSANE_EXPORT KSaneCore : public QObject
{
    Q_OBJECT
    friend class KSaneCorePrivate;

public:
    struct DeviceInfo
    {
        QString name;     /* unique device name */
        QString vendor;   /* device vendor string */
        QString model;    /* device model name */
        QString type;     /* device type (e.g., "flatbed scanner") */
    };

    /** This constructor initializes the private class variables and starts a
     * thread to search for scanner devices. The result is signaled with availableDevices() */
    KSaneCore(QObject* parent=0);

    /** Standard destructor */
    ~KSaneCore();

    /**
     * Get the list of available scanning devices. Connect to availableDevices()
     * which is fired once the devices are known.
     * \param rescanDeviceList do a rescan of the devices even if a device list
     * already is in memory.
     * \note All sane backends do not support rescanning for devices, which means
     * that the list might not be updated even if a new scanner is connected. */
    void requestDeviceList(bool rescanDeviceList);

Q_SIGNALS:
    /**
     * This signal is emitted every time the device list is updated or
     * after initGetDeviceList() is called.
     * param deviceList is a QList of KSaneCore::DeviceInfo that contain the
     * device name, model, vendor and type of the attached scanners.
     * \note The list is only a snapshot of the current available devices. Devices
     * might be added or removed/opened after the signal is emitted.
     */
    void availableDevices(const QList<KSaneCore::DeviceInfo> &deviceList);

private:

    KSaneCorePrivate * const d;
};

#endif
