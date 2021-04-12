/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_INTEGER_OPTION_H
#define KSANE_INTEGER_OPTION_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneIntegerOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneIntegerOption(const SANE_Handle handle, const int index);

    void readValue() override;

    QVariant minimumValue() const override;
    QVariant maximumValue() const override;
    QVariant stepValue() const override;
    QVariant value() const override;
    QString valueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    int            m_iVal;
};

}  // NameSpace KSaneIface

#endif // KSANE_INTEGER_OPTION_H
