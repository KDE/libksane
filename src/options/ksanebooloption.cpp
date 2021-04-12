/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */
// Local includes
#include "ksanebooloption.h"

#include <QtCore/QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneBoolOption::KSaneBoolOption(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_checked(false)
{
    m_optionType = KSaneOption::TypeBool;
}

bool KSaneBoolOption::setValue(const QVariant &value)
{
    if (state() == StateHidden) {
        return false;
    }
    
    bool toggled = value.toBool();
    
    if (m_checked != toggled) {
        unsigned char data[4];

        m_checked = toggled;
        fromSANE_Word(data, (toggled) ? 1 : 0);
        writeData(data);
        Q_EMIT valueChanged(m_checked);
    }
    return true;
}

void KSaneBoolOption::readValue()
{
    if (state() == StateHidden) {
        return;
    }

    // read the current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }
    bool old = m_checked;
    m_checked = (toSANE_Word(data.data()) != 0) ? true : false;

    if ((old != m_checked) && ((m_optDesc->cap & SANE_CAP_SOFT_SELECT) == 0)) {
        Q_EMIT valueChanged(m_checked);
    }
}

QVariant KSaneBoolOption::getValue() const
{
    if (state() == StateHidden) {
        return QVariant();
    }
    return m_checked;
}

QString KSaneBoolOption::getValueAsString() const
{
    if (state() == StateHidden) {
        return QString();
    }
    if (m_checked) {
        return QStringLiteral("true");
    } else {
        return QStringLiteral("false");
    }
}

}  // NameSpace KSaneIface
