/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanebatchmodeoption.h"

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneBatchModeOption::KSaneBatchModeOption()
{
    m_optionType = KSaneOption::TypeBool;
}

KSaneOption::KSaneOptionState KSaneBatchModeOption::state() const
{
    return KSaneOption::StateActive;
}

QString KSaneBatchModeOption::name() const
{
    return BatchModeOptionName;
}

QString KSaneBatchModeOption::title() const
{
    return i18n("Batch mode with time delay");
}

QString KSaneBatchModeOption::description() const
{
    return i18n("Enables batch mode scanning. Continues scanning after a delay until canceled.");
}

bool KSaneBatchModeOption::setValue(const QVariant &value)
{
    const bool toggled = value.toBool();

    if (m_checked != toggled) {
        m_checked = toggled;
        Q_EMIT valueChanged(m_checked);
    }
    return true;
}

QVariant KSaneBatchModeOption::value() const
{
    return m_checked;
}

QString KSaneBatchModeOption::valueAsString() const
{
    if (state() == KSaneOption::StateHidden) {
        return QString();
    }
    if (m_checked) {
        return QStringLiteral("true");
    } else {
        return QStringLiteral("false");
    }
}

}  // NameSpace KSaneIface
