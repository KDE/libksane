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

#ifndef KSANE_OPT_COMBO_H
#define KSANE_OPT_COMBO_H

#include "ksaneoption.h"

#include <utility>

namespace KSaneIface
{

class KSaneOptCombo : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptCombo(const SANE_Handle handle, const int index);

    void readValue() override;

    bool getMinValue(float &max) override;
    bool getValue(float &val) override;
    bool getValue(QString &val) override;
    QVariantList getEntryList() const override;
    
public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    std::pair<QString, QString> getSaneComboString(unsigned char *data) const;
    bool setValue(float value);
    bool setValue(const QString &value);

    QVariant       m_currentValue;
};

}  // NameSpace KSaneIface

#endif
