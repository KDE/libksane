/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_ACTION_OPTION_H
#define KSANE_ACTION_OPTION_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneActionOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneActionOption(const SANE_Handle handle, const int index);

public Q_SLOTS:
    bool setValue(const QVariant &value) override;
};

}  // NameSpace KSaneIface

#endif // KSANE_ACTION_OPTION_H
