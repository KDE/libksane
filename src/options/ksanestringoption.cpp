/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */
// Local includes
#include "ksanestringoption.h"

#include <QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneStringOption::KSaneStringOption(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index)
{
    m_optionType = KSaneOption::TypeString;
}

bool KSaneStringOption::setValue(const QVariant &val)
{
    if (state() == StateHidden) {
        return false;
    }
    QString text = val.toString();
    QString tmp;
    tmp += text.leftRef(m_optDesc->size);
    if (tmp != text) {
        writeData(tmp.toLatin1().data());
        Q_EMIT valueChanged(tmp);
    }
    return true;
}

void KSaneStringOption::readValue()
{
    if (state() == StateHidden) {
        return;
    }

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    m_string = QString::fromUtf8(reinterpret_cast<char *>(data.data()));

    Q_EMIT valueChanged(m_string);
}

QVariant KSaneStringOption::getValue() const
{
    return QVariant(m_string);
}

QString KSaneStringOption::getValueAsString() const
{
    if (state() == StateHidden) {
        return QString();
    }
    return m_string;
}

}  // NameSpace KSaneIface
