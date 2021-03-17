/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

// Local includes
#include "labeledfslider.h"

#define FLOAT_MULTIP 32768.0
#define TO_FLOAT(v) ((float)v / FLOAT_MULTIP)
#define TO_FIX(v) ((int)(v * FLOAT_MULTIP))

namespace KSaneIface
{

LabeledFSlider::LabeledFSlider(QWidget *parent, const QString &ltext,
                               float min, float max, float step)
    : KSaneOptionWidget(parent, ltext)
{
    initFSlider(min, max, step);
}

LabeledFSlider::LabeledFSlider(QWidget *parent, KSaneOption *option)
    : KSaneOptionWidget(parent, option)
{
    float maxValueF = 0.0;
    option->getMaxValue(maxValueF);
    float minValueF = 0.0;
    option->getMinValue(minValueF);
    float stepValueF = 0.0;
    option->getStepValue(stepValueF);
    initFSlider(minValueF, maxValueF , stepValueF);
    
    QString unitSuffix;
    KSaneOption::KSaneOptionUnit unit = option->getUnit();
    switch (unit) {
    case KSaneOption::UnitPixel: 
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit", " Pixels");
        break;
    case KSaneOption::UnitBit:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit", " Bits");
        break;
    case KSaneOption::UnitMilliMeter: 
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Millimeter)", " mm");
        break;
    case KSaneOption::UnitDPI:  
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Dots Per Inch)", " DPI");
        break;
    case KSaneOption::UnitPercent: 
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Percentage)", " %");
        break;
    case KSaneOption::UnitMicroSecond:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Microseconds)", " µs");
        break;
    default: 
        unitSuffix = QString();
        break;
    }
    setSuffix(unitSuffix);
    
    setLabelText(option->title());
    setToolTip(option->description());
    connect(this, &LabeledFSlider::valueChanged, option, &KSaneOption::setValue);
    connect(option, &KSaneOption::valueChanged, this, &LabeledFSlider::setValue);
    float valueF = 0.0;
    option->getValue(valueF);
    setValue(valueF);
}

void LabeledFSlider::initFSlider(float minValueF, float maxValueF, float stepValueF)
{
    int imin = TO_FIX(minValueF);
    int imax = TO_FIX(maxValueF);
    m_istep = TO_FIX(stepValueF);
    m_fstep = stepValueF;
    if (m_istep == 0) {
        m_istep = 1;
        m_fstep = TO_FLOAT(m_istep);
    }

    //std::cout << "min=" << min << ", max=" << max << ", m_fstep="<<m_fstep<<std::endl;
    //std::cout << "imin=" << imin << ", imax=" << imax << ", m_istep="<<m_istep<<std::endl;
    m_slider = new QSlider(this);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setMinimum(imin);
    m_slider->setMaximum(imax);
    m_slider->setSingleStep(m_istep);
    m_slider->setValue(imin);

    m_spinb = new QDoubleSpinBox(this);
    m_spinb->setMinimum(minValueF);
    m_spinb->setMaximum(maxValueF);
    m_spinb->setSingleStep(m_fstep);
    int decimals = 0;
    float tmp_step = m_fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) {
            break;
        }
    }
    m_spinb->setDecimals(decimals);
    m_spinb->setValue(maxValueF);
    //m_spinb->setMinimumWidth(m_spinb->sizeHint().width()+35);
    m_spinb->setMinimumWidth(m_spinb->sizeHint().width());
    m_spinb->setAlignment(Qt::AlignRight);
    m_spinb->setValue(minValueF);

    m_label->setBuddy(m_spinb);

    connect(m_spinb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, QOverload<double>::of(&LabeledFSlider::syncValues));
    connect(m_slider, QOverload<int>::of(&QSlider::valueChanged), this, QOverload<int>::of(&LabeledFSlider::syncValues));
    connect(m_slider, &QSlider::sliderReleased, this, &LabeledFSlider::fixValue);

    m_layout->addWidget(m_slider, 0, 2);
    m_layout->addWidget(m_spinb, 0, 1);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);
    m_layout->activate();
}

LabeledFSlider::~LabeledFSlider()
{
}

float LabeledFSlider::value() const
{
    return (float)m_spinb->value();
}

float LabeledFSlider::step() const
{
    return m_fstep;
}

void LabeledFSlider::setSuffix(const QString &text)
{
    m_spinb->setSuffix(text);
}

void LabeledFSlider::setRange(float min, float max)
{
    //qCDebug(KSANE_LOG) << "min,max(" << m_spinb->minimum() << " - " << m_spinb->maximum();
    //qCDebug(KSANE_LOG) << ") -> (" << min << " - " << max << ")" << std::endl;
    int imin = TO_FIX(min);
    int imax = TO_FIX(max);
    m_slider->setRange(imin, imax);
    //std::cout << "fixedmin=" << imin << " fixedmax=" << imax << std::endl;

    m_spinb->setRange(min, max);
}

void LabeledFSlider::setStep(float step)
{
    m_istep = TO_FIX(step);
    m_fstep = step;
    if (m_istep == 0) {
        m_istep = 1;
        m_fstep = TO_FLOAT(m_istep);
    }
    m_slider->setSingleStep(m_istep);
    m_spinb->setSingleStep(m_fstep);

    int decimals = 0;
    float tmp_step = m_fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) {
            break;
        }
    }
    m_spinb->setDecimals(decimals);
}

void LabeledFSlider::setValue(const QVariant &value)
{
    bool ok;
    float newValue = value.toFloat(&ok);
    if (!ok) {
        return;
    }
    int ivalue = TO_FIX(newValue);

    if (((newValue - m_spinb->value()) > m_fstep) || ((m_spinb->value() - newValue) > m_fstep)) {
        m_spinb->setValue(newValue);
    } else if (ivalue != m_slider->value()) {
        m_slider->setValue(ivalue);
    }
}

void LabeledFSlider::syncValues(int ivalue)
{
    double value = TO_FLOAT(ivalue);
    if (((value - m_spinb->value()) > m_fstep) || ((m_spinb->value() - value) > m_fstep)) {
        m_spinb->setValue(value);
    } else if (ivalue != m_slider->value()) {
        m_slider->setValue(ivalue);
    } else {
        Q_EMIT valueChanged(value);
    }
}

void LabeledFSlider::syncValues(double value)
{
    int ivalue = TO_FIX(value);
    if (((value - m_spinb->value()) > m_fstep) || ((m_spinb->value() - value) > m_fstep)) {
        m_spinb->setValue(value);
    } else if (ivalue != m_slider->value()) {
        m_slider->setValue(ivalue);
    } else {
        Q_EMIT valueChanged((float)value);
    }
}

void LabeledFSlider::fixValue()
{
    //ensure that the value step is followed also for the m_slider
    int rest = (m_slider->value() - m_slider->minimum()) % m_istep;

    if (rest != 0) {
        if (rest > (m_istep / 2)) {
            m_slider->setValue(m_slider->value() + (m_istep - rest));
        } else {
            m_slider->setValue(m_slider->value() - rest);
        }
        m_spinb->setValue(TO_FLOAT(m_slider->value()));
    }
}

}  // NameSpace KSaneIface
