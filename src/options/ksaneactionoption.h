/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_OPT_BUTTON_H
#define KSANE_OPT_BUTTON_H

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

#endif // KSANE_BUTTON_OPT_H
