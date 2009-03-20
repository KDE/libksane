/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2009 by Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
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

#include "ksane_find_devices_thread.h"
#include "ksane_find_devices_thread.moc"

// KDE includes.
#include <KDebug>


namespace KSaneIface
{

FindSaneDevicesThread::FindSaneDevicesThread(QObject *parent)
     : QThread(parent)
{
}

FindSaneDevicesThread::~FindSaneDevicesThread()
{
}


void FindSaneDevicesThread::run()
{
    QString tmp;
    int     i = 0;

    status = sane_get_devices(&dev_list, SANE_FALSE);

    while(dev_list[i] != 0) {
        tmp = QString(dev_list[i]->vendor);
        tmp += " : " + QString(dev_list[i]->model);
        tmp += "\n " + QString(dev_list[i]->name);
//         kDebug(51004) << "FindSaneDevicesThread::found device: " << dev_list[i]->name;
        devices_map.insert( dev_list[i]->name, tmp );
        i++;
    }
}

void FindSaneDevicesThread::getDevicesList(QMap<QString, QString> &devices_list) {
    devices_list = devices_map;
}

}