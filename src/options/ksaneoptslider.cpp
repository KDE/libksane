/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoptslider.h"

#include <QVarLengthArray>

static const int KSW_INT_MAX = 2147483647;
static const int KSW_INT_MIN = -2147483647 - 1; // prevent warning

namespace KSaneIface
{

KSaneOptSlider::KSaneOptSlider(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_iVal(0)
{
    m_optionType = KSaneOption::TypeInteger;
}

void KSaneOptSlider::readValue()
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

    m_iVal = toSANE_Word(data.data());

    Q_EMIT valueChanged(m_iVal);
}

bool KSaneOptSlider::getMinValue(float &val)
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        val = (float)m_optDesc->constraint.range->min;
    } else {
        val = (float)KSW_INT_MIN;
    }
    return true;
}

bool KSaneOptSlider::getMaxValue(float &val)
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        val = (float)m_optDesc->constraint.range->max;
    } else {
        val = (float)KSW_INT_MAX;
    }
    return true;
}

bool KSaneOptSlider::getStepValue(float &val)
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        val = (float)m_optDesc->constraint.range->quant;
    } else {
        val = 1;
    }
    return true;
}

bool KSaneOptSlider::getValue(float &val)
{
    if (state() == StateHidden) {
        return false;
    }
    val = (float)m_iVal;
    return true;
}

bool KSaneOptSlider::getValue(QString &val)
{
    if (state() == StateHidden) {
        return false;
    }
    val = QString::number(m_iVal);
    return true;
}

bool KSaneOptSlider::setValue(const QVariant &val)
{
    bool ok;
    int newValue = val.toInt(&ok);
    if (ok && newValue != m_iVal) {
        unsigned char data[4];
        m_iVal = newValue;
        fromSANE_Word(data, newValue);
        writeData(data);
        Q_EMIT valueChanged(m_iVal);
    }
    return ok;
}

}  // NameSpace KSaneIface
