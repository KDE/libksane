/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2021-04-6
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_DOUBLE_OPTION_H
#define KSANE_DOUBLE_OPTION_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneDoubleOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneDoubleOption(const SANE_Handle handle, const int index);

    void readValue() override;
    void readOption() override;

    QVariant getMinValue() const override;
    QVariant getMaxValue() const override;
    QVariant getStepValue() const override;
    QVariant getValue() const override;
    QString getValueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    double  m_value = 0;
    double  m_minChange = 0.0001;
};

}  // NameSpace KSaneIface

#endif // KSANE_FLOAT_OPTION_H
