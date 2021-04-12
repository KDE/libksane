/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_VALUELIST_OPTION_H
#define KSANE_VALUELIST_OPTION_H

#include "ksaneoption.h"

#include <utility>

namespace KSaneIface
{

class KSaneListOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneListOption(const SANE_Handle handle, const int index);

    void readValue() override;

    QVariant getMinValue() const override;
    QVariant getValue() const override;
    QString getValueAsString() const override;
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

#endif // KSANE_VALUELIST_OPTION_H
