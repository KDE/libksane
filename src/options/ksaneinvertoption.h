/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_INVERT_OPTION_H
#define KSANE_INVERT_OPTION_H

#include "ksaneoption.h"

namespace KSaneIface
{

static const QString InvertColorsOptionName = QStringLiteral("KSane::InvertColors");    
    
class KSaneInvertOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneInvertOption();

    QVariant getValue() const override;
    QString getValueAsString() const override;
    
    KSaneOptionState state() const override;
    QString name() const override;
    QString title() const override;
    QString description() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    bool m_checked = false;
};  

}  // NameSpace KSaneIface

#endif // KSANE_INVERT_OPTION_H
