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

#ifndef KSANE_OPT_CHECKBOX_H
#define KSANE_OPT_CHECKBOX_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneOptCheckBox : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptCheckBox(const SANE_Handle handle, const int index);

    void readValue() override;

    QVariant getValue() const override;
    QString getValueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

Q_SIGNALS:
    void buttonPressed(const QString &optionName, const QString &optionLabel, bool pressed);

private:
    bool             m_checked;
};

}  // NameSpace KSaneIface

#endif
