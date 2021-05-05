/* ============================================================
 *
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */


#include "ksaneinternaloption.h"
#include "ksaneoption_p.h"
#include "ksanebaseoption.h"


namespace KSaneIface
{

KSaneInternalOption::KSaneInternalOption(KSaneBaseOption *option, QObject *parent) : KSaneOption(parent)
{
    d->option = option;
    connect(d->option, &KSaneBaseOption::optionReloaded, this, &KSaneOption::optionReloaded);
    connect(d->option, &KSaneBaseOption::valueChanged, this, &KSaneOption::valueChanged);
    connect(d->option, &KSaneBaseOption::destroyed, [=]() { d->option = nullptr; } );
}

}  // NameSpace KSaneIface
