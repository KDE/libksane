/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_BATCH_OPTION_H
#define KSANE_BATCH_OPTION_H

#include "ksanebaseoption.h"

namespace KSaneIface
{

static const QString BatchModeOptionName = QStringLiteral("KSane::BatchMode");

class KSaneBatchModeOption : public KSaneBaseOption
{
    Q_OBJECT

public:
    KSaneBatchModeOption();

    KSaneOption::KSaneOptionState state() const override;
    QString name() const override;
    QString title() const override;
    QString description() const override;

    QVariant value() const override;
    QString valueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    bool m_checked = false;
};

}  // NameSpace KSaneIface

#endif // KSANE_BATCH_OPTION_H
