/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2012-05-25
 * Description : Sane interface for KDE
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

#ifndef KSaneAccessSingleton_H
#define KSaneAccessSingleton_H

#include "KSaneDevice.h"

#include <QObject>
#include <QMutex>

extern "C"
{
    #include <sane/saneopts.h>
    #include <sane/sane.h>
}

class KSaneAccessSingleton: public QObject
{
    Q_OBJECT
public:
    static KSaneAccessSingleton *getInstance();

    QMutex accessMutex;

    static void authorization(SANE_String_Const resource, SANE_Char *username, SANE_Char *password);
    void setDeviceAuth(KSaneDevice *device, const QString &username, const QString &password);
    void clearDeviceAuth(KSaneDevice *device);

    void reserveSaneInterface(KSaneDevice *requester);
    void releaseSaneInterface(KSaneDevice *requester);

private:
    struct AuthStruct {
        KSaneDevice *device;
        QString username;
        QString password;
    };

    KSaneAccessSingleton();
    ~KSaneAccessSingleton();
    int m_reserveCount;
    QList<AuthStruct> m_authList;
};


#endif
