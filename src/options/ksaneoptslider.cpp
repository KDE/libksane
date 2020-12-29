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

#include "labeledslider.h"

#include <QtCore/QVarLengthArray>

static const int KSW_INT_MAX = 2147483647;
static const int KSW_INT_MIN = -2147483647 - 1; // prevent warning

namespace KSaneIface
{

KSaneOptSlider::KSaneOptSlider(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_slider(nullptr), m_iVal(0)
{
}

void KSaneOptSlider::createWidget(QWidget *parent)
{
    if (m_widget) {
        return;
    }

    m_widget = m_slider = new LabeledSlider(parent, QString(), KSW_INT_MIN, KSW_INT_MAX, 1);
    readOption();
    m_widget->setToolTip(sane_i18n(m_optDesc->desc));
    connect(m_slider, &LabeledSlider::valueChanged, this, &KSaneOptSlider::sliderChanged);
    readValue();
}

void KSaneOptSlider::readValue()
{
    if (state() == STATE_HIDDEN) {
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
    if ((m_slider != nullptr) && (m_slider->value() != m_iVal)) {
        m_slider->setValue(m_iVal);
    }
    Q_EMIT fValueRead((float)m_iVal);
}

void KSaneOptSlider::readOption()
{
    KSaneOption::readOption();

    if (!m_slider) {
        return;
    }

    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        m_slider->setRange(m_optDesc->constraint.range->min, m_optDesc->constraint.range->max);
        m_slider->setStep(m_optDesc->constraint.range->quant);
    } else {
        m_slider->setRange(KSW_INT_MIN, KSW_INT_MAX);
        m_slider->setStep(1);
    }
    m_slider->setSuffix(unitString());
    m_slider->setLabelText(sane_i18n(m_optDesc->title));
}

void KSaneOptSlider::sliderChanged(int val)
{
    if (val == m_iVal) {
        return;
    }
    unsigned char data[4];
    m_iVal = val;
    fromSANE_Word(data, val);
    writeData(data);
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

bool KSaneOptSlider::getValue(float &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    val = (float)m_iVal;
    return true;
}

bool KSaneOptSlider::setValue(float val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    sliderChanged((int)val);
    readValue();
    return true;
}

bool KSaneOptSlider::getValue(QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    val = QString::number(m_iVal);
    return true;
}

bool KSaneOptSlider::setValue(const QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    sliderChanged(val.toInt());
    readValue();
    return true;
}

bool KSaneOptSlider::hasGui()
{
    return true;
}

}  // NameSpace KSaneIface
