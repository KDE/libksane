/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2012-05-24
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2012 by Kare Sars <kare.sars@iki.fi>
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

#include "KSaneAccessSingleton.h"
#include "KSaneAccessSingleton.moc"
#include "ksane_auth.h"
#include <QMutexLocker>

#include <KDebug>

// Sane includes
extern "C"
{
    #include <sane/saneopts.h>
    #include <sane/sane.h>
}

static KSaneAccessSingleton *s_instance = 0;
static QMutex s_mutex;

KSaneAccessSingleton *KSaneAccessSingleton::getInstance()
{
    QMutexLocker loacker(&s_mutex);

    if (s_instance == 0) {
        s_instance = new KSaneAccessSingleton();
    }

    return s_instance;
}


KSaneAccessSingleton::KSaneAccessSingleton(): QObject(),
m_reserveCount(0)
{
    // only accessed from getInstance() -> no mutex need
    SANE_Int    version;
    SANE_Status status;

    status = sane_init(&version, &KSaneAuth::authorization);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "libksane: sane_init() failed:" << sane_strstatus(status);
    }
    else {
        //kDebug() << "Sane Version = "
        //         << SANE_VERSION_MAJOR(version) << "."
        //         << SANE_VERSION_MINORparent(version) << "."
        //         << SANE_VERSION_BUILD(version);
    }
}

KSaneAccessSingleton::~KSaneAccessSingleton()
{
    // only accessed from releaseSaneInterface -> no mutex need
    s_instance = 0;
}

void KSaneAccessSingleton::reserveSaneInterface(KSaneDevice */*requester*/)
{
    QMutexLocker lock(&s_mutex);
    m_reserveCount++;
}

void KSaneAccessSingleton::releaseSaneInterface(KSaneDevice */*requester*/)
{
    QMutexLocker lock(&s_mutex);
    m_reserveCount--;
    if (m_reserveCount == 0) {
        delete this;
    }
}

/** static function called by sane_open to get authorization from user */
void KSaneAccessSingleton::authorization(SANE_String_Const resource, SANE_Char *username, SANE_Char *password)
{
    kDebug() << resource;
    // This is vague in the standard... what can I find in the resource string?
    // I have found that "resource contains the backend name + "$MD5$....."
    // it does not contain unique identifiers like ":libusb:001:004"
    // -> remove $MD5 and later before comparison...
    QString res(resource);
    int end = res.indexOf("$MD5$");
    res = res.left(end);
    kDebug() << res;
    
    QList<AuthStruct> list = getInstance()->m_authList;
    for (int i=0; i<list.size(); i++) {
        kDebug() << res << list.at(i).device->info().name;
        if (list.at(i).device->info().name.contains(res)) {
            qstrncpy(username, list.at(i).username.toLocal8Bit(), SANE_MAX_USERNAME_LEN);
            qstrncpy(password, list.at(i).password.toLocal8Bit(), SANE_MAX_PASSWORD_LEN);
            break;
        }
    }
}


