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
// Local includes
#include "ksaneoptfslider.h"

#include <QVarLengthArray>

#include <ksane_debug.h>

static const float FIXED_MAX = 32767.0;
static const float FIXED_MIN = -32768.0;
static const float MIN_FIXED_STEP = 0.0001;
namespace KSaneIface
{

KSaneOptFSlider::KSaneOptFSlider(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_fVal(0), m_minChange(MIN_FIXED_STEP)
{
    m_optionType = KSaneOption::TypeFloat;
}

void KSaneOptFSlider::readOption()
{
    KSaneOption::readOption();

    float step = MIN_FIXED_STEP;
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        step = SANE_UNFIX(m_optDesc->constraint.range->quant);
        if (step < MIN_FIXED_STEP) {
            step = MIN_FIXED_STEP;
        }
    } 
    m_minChange = step;
}

void KSaneOptFSlider::readValue()
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

    m_fVal = SANE_UNFIX(toSANE_Word(data.data()));

    Q_EMIT valueChanged(m_fVal);
}

bool KSaneOptFSlider::setValue(const QVariant &value)
{
    if (state() == StateHidden) {
        return false;
    }
    bool ok;
    float newValue = value.toFloat(&ok);
    if (ok && (((newValue - m_fVal) >= m_minChange) || ((m_fVal - newValue) >= m_minChange)) ) {
        unsigned char data[4];
        SANE_Word fixed;
        //qCDebug(KSANE_LOG) <<m_optDesc->name << fVal << "!=" << val;
        m_fVal = newValue;
        fixed = SANE_FIX(newValue);
        fromSANE_Word(data, fixed);
        writeData(data);
        Q_EMIT valueChanged(m_fVal);
    }
    return ok;
}

QVariant KSaneOptFSlider::getMinValue() const
{
    QVariant value;
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        value = SANE_UNFIX(m_optDesc->constraint.range->min);
    } else {
        value = FIXED_MIN;
    }
    return value;
}

QVariant KSaneOptFSlider::getMaxValue() const
{
    QVariant value;
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        value = SANE_UNFIX(m_optDesc->constraint.range->max);
    } else {
        value = FIXED_MAX;
    }
    return value;
}

QVariant KSaneOptFSlider::getStepValue() const
{
    QVariant value;
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        value = SANE_UNFIX(m_optDesc->constraint.range->quant);
    } else {
        value = 0.0001;
    }
    return value;
}

QVariant KSaneOptFSlider::getValue() const
{
    if (state() == StateHidden) {
        return QVariant();
    }
    return QVariant(m_fVal);
}

QString KSaneOptFSlider::getValueAsString() const
{
    if (state() == StateHidden) {
        return QString();
    }
    return QString::number(m_fVal, 'F', 6);
}
    
}  // NameSpace KSaneIface
