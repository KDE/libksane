/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_BOOL_OPTION_H
#define KSANE_BOOL_OPTION_H

#include "ksanebaseoption.h"

namespace KSaneIface
{

class KSaneBoolOption : public KSaneBaseOption
{
    Q_OBJECT

public:
    KSaneBoolOption(const SANE_Handle handle, const int index);

    void readValue() override;

    QVariant value() const override;
    QString valueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    bool m_checked = false;
};

}  // NameSpace KSaneIface

#endif // KSANE_BOOL_OPTION_H
