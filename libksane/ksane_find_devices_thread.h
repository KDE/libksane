/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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

#ifndef FIND_SANE_DEVICES_H
#define FIND_SANE_DEVICES_H

// #include "ksane.h"

#include <QObject>
#include <QThread>
#include <QMap>

// Sane includes.
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

namespace KSaneIface
{

class FindSaneDevicesThread : public QThread {
    Q_OBJECT

    public:
        FindSaneDevicesThread(QObject *parent);
        ~FindSaneDevicesThread();
        void run();
        void getDevicesList(QMap<QString, QString> &devices_list);

    private:
        SANE_Status         status;
        SANE_Device const   **dev_list;
        QMap<QString, QString> devices_map;
};

}

#endif // FIND_SANE_DEVICES_H
