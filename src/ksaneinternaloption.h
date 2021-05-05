/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_INTERNAL_OPTION_H
#define KSANE_INTERNAL_OPTION_H

#include "ksaneoption.h"
#include "ksanebaseoption.h"

namespace KSaneIface
{

class KSaneInternalOption : public KSaneOption
{
    Q_OBJECT

public:

    KSaneInternalOption(KSaneBaseOption *option, QObject *parent = nullptr);

};
    
}  // NameSpace KSaneIface

#endif // KSANE_INTERNAL_OPTION_H

