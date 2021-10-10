/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanebatchdelayoption.h"
#include <ksane_debug.h>

namespace KSaneIface
{

KSaneBatchDelayOption::KSaneBatchDelayOption()
{
    m_optionType = KSaneOption::TypeInteger;
}

KSaneOption::KSaneOptionState KSaneBatchDelayOption::state() const
{
    return KSaneOption::StateActive;;
}

QString KSaneBatchDelayOption::name() const
{
    return BatchDelayOptionName;
}

QString KSaneBatchDelayOption::title() const
{
    return i18n("Batch mode time delay");
}

QString KSaneBatchDelayOption::description() const
{
    return i18n("Specify the time delay between each scan when batch mode is enabled.");
}

QVariant KSaneBatchDelayOption::minimumValue() const
{
    return 0;
}

QVariant KSaneBatchDelayOption::maximumValue() const
{
    return 300;
}

QVariant KSaneBatchDelayOption::stepValue() const
{
    return 1;
}

QVariant KSaneBatchDelayOption::value() const
{
    return m_delayValue;
}

QString KSaneBatchDelayOption::valueAsString() const
{
    return QString::number(m_delayValue);
}

KSaneOption::KSaneOptionUnit KSaneBatchDelayOption::valueUnit() const
{
    return KSaneOption::UnitSecond;
}

bool KSaneBatchDelayOption::setValue(const QVariant &val)
{
    bool ok;
    int newValue = val.toInt(&ok);
    if (ok && newValue != m_delayValue) {
        m_delayValue = newValue;
        Q_EMIT valueChanged(m_delayValue);
    }
    return ok;
}

}  // NameSpace KSaneIface
