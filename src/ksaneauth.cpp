/* ============================================================
*
* SPDX-FileCopyrightText: 2010 Kare Sars <kare dot sars at iki dot fi>
* SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#include "ksaneauth.h"

// Qt includes
#include <QMutex>
#include <QMutexLocker>
#include <QList>

#include <ksane_debug.h>

namespace KSaneIface
{

static KSaneAuth *s_instance = nullptr;
Q_GLOBAL_STATIC(QMutex, s_mutex)

struct KSaneAuth::Private {
    struct AuthStruct {
        QString resource;
        QString username;
        QString password;
    };

    QList<AuthStruct> authList;
};

KSaneAuth *KSaneAuth::getInstance()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMutexLocker<QMutex> locker(s_mutex);
#else
    QMutexLocker locker(s_mutex);
#endif

    if (s_instance == nullptr) {
        s_instance = new KSaneAuth();
    }
    return s_instance;
}

KSaneAuth::KSaneAuth() : d(new Private) {}

KSaneAuth::~KSaneAuth()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMutexLocker<QMutex> locker(s_mutex);
#else
    QMutexLocker locker(s_mutex);
#endif
    d->authList.clear();
    delete d;
}

void KSaneAuth::setDeviceAuth(const QString &resource, const QString &username, const QString &password)
{
    // This is a short list so we do not need a QMap...
    int i;
    for (i = 0; i < d->authList.size(); i++) {
        if (resource == d->authList.at(i).resource) {
            // update the existing node
            d->authList[i].username = username;
            d->authList[i].password = password;
            break;
        }
    }
    if (i == d->authList.size()) {
        // Add a new list node
        Private::AuthStruct tmp;
        tmp.resource = resource;
        tmp.username = username;
        tmp.password = password;
        d->authList << tmp;
    }
}

void KSaneAuth::clearDeviceAuth(const QString &resource)
{
    // This is a short list so we do not need a QMap...
    for (int i = 0; i < d->authList.size(); i++) {
        if (resource == d->authList.at(i).resource) {
            d->authList.removeAt(i);
            return;
        }
    }
}

/** static function called by sane_open to get authorization from user */
void KSaneAuth::authorization(SANE_String_Const resource, SANE_Char *username, SANE_Char *password)
{
    qCDebug(KSANE_LOG) << resource;
    // This is vague in the standard... what can I find in the resource string?
    // I have found that "resource contains the backend name + "$MD5$....."
    // it does not contain unique identifiers like ":libusb:001:004"
    // -> remove $MD5 and later before comparison...
    QString res = QString::fromUtf8(resource);
    int end = res.indexOf(QStringLiteral("$MD5$"));
    res = res.left(end);
    qCDebug(KSANE_LOG) << res;

    QList<Private::AuthStruct> list = getInstance()->d->authList;
    for (int i = 0; i < list.size(); i++) {
        qCDebug(KSANE_LOG) << res << list.at(i).resource;
        if (list.at(i).resource.contains(res)) {
            qstrncpy(username, list.at(i).username.toLocal8Bit().constData(), SANE_MAX_USERNAME_LEN);
            qstrncpy(password, list.at(i).password.toLocal8Bit().constData(), SANE_MAX_PASSWORD_LEN);
            break;
        }
    }
}

}

