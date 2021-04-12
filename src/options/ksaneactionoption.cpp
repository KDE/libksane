/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneactionoption.h"

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneActionOption::KSaneActionOption(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index)
{
    m_optionType = KSaneOption::TypeAction;
}

bool KSaneActionOption::setValue(const QVariant &)
{
    unsigned char data[4];
    writeData(data);
    return true;
}

}  // NameSpace KSaneIface
