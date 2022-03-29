/* ============================================================
*
* SPDX-FileCopyrightText: 2010 Kare Sars <kare dot sars at iki dot fi>
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#ifndef KSANE_AUTH_H
#define KSANE_AUTH_H

// Qt includes
#include <QString>

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

namespace KSaneIface
{

/**
 * Sane authentication helpers.
 */
class KSaneAuth
{
public:
    static KSaneAuth *getInstance();
    ~KSaneAuth();

    void setDeviceAuth(const QString &resource, const QString &username, const QString &password);
    void clearDeviceAuth(const QString &resource);
    static void authorization(SANE_String_Const resource, SANE_Char *username, SANE_Char *password);

private:
    KSaneAuth();
    struct Private;
    Private *const d;
};

}

#endif
