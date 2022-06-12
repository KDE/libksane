/*
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

// Local includes
#include "labeledfslider.h"

//KDE includes

#include <KLocalizedString>

#define FLOAT_MULTIP 32768.0
#define TO_DOUBLE(v) (static_cast<double>(v) / FLOAT_MULTIP)
#define TO_FIX(v) (static_cast<int>(v * FLOAT_MULTIP))

namespace KSaneIface
{

LabeledFSlider::LabeledFSlider(QWidget *parent, const QString &ltext,
                               double min, double max, double step)
    : KSaneOptionWidget(parent, ltext)
{
    initFSlider(min, max, step);
}

LabeledFSlider::LabeledFSlider(QWidget *parent, KSaneCore::Option *option)
    : KSaneOptionWidget(parent, option)
{
    double maxValue = option->maximumValue().toDouble();
    double minValue = option->minimumValue().toDouble();
    double stepValue = option->stepValue().toDouble();
    initFSlider(minValue, maxValue , stepValue);

    QString unitSuffix;
    KSaneCore::Option::OptionUnit unit = option->valueUnit();
    switch (unit) {
    case KSaneCore::Option::UnitPixel:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit", " Pixels");
        break;
    case KSaneCore::Option::UnitBit:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit", " Bits");
        break;
    case KSaneCore::Option::UnitMilliMeter:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Millimeter)", " mm");
        break;
    case KSaneCore::Option::UnitDPI:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Dots Per Inch)", " DPI");
        break;
    case KSaneCore::Option::UnitPercent:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Percentage)", " %");
        break;
    case KSaneCore::Option::UnitMicroSecond:
        unitSuffix = i18nc("Double numbers. SpinBox parameter unit (Microseconds)", " µs");
        break;
    case KSaneCore::Option::UnitSecond:
        unitSuffix = i18nc("SpinBox parameter unit (seconds), float", " s");
        break;
    default:
        unitSuffix = QString();
        break;
    }
    setSuffix(unitSuffix);

    setLabelText(option->title());
    setToolTip(option->description());
    connect(this, &LabeledFSlider::valueChanged, option, &KSaneCore::Option::setValue);
    connect(option, &KSaneCore::Option::valueChanged, this, &LabeledFSlider::setValue);
    double value = option->value().toDouble();
    setValue(value);
}

void LabeledFSlider::initFSlider(double minValue, double maxValue, double stepValue)
{
    int imin = TO_FIX(minValue);
    int imax = TO_FIX(maxValue);
    m_istep = TO_FIX(stepValue);
    m_fstep = stepValue;
    if (m_istep == 0) {
        m_istep = 1;
        m_fstep = TO_DOUBLE(m_istep);
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
    m_spinb->setMinimum(minValue);
    m_spinb->setMaximum(maxValue);
    m_spinb->setSingleStep(m_fstep);
    int decimals = 0;
    double tmp_step = m_fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) {
            break;
        }
    }
    m_spinb->setDecimals(decimals);
    m_spinb->setValue(maxValue);
    //m_spinb->setMinimumWidth(m_spinb->sizeHint().width()+35);
    m_spinb->setMinimumWidth(m_spinb->sizeHint().width());
    m_spinb->setAlignment(Qt::AlignRight);
    m_spinb->setValue(minValue);

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

double LabeledFSlider::value() const
{
    return m_spinb->value();
}

double LabeledFSlider::step() const
{
    return m_fstep;
}

void LabeledFSlider::setSuffix(const QString &text)
{
    m_spinb->setSuffix(text);
}

void LabeledFSlider::setRange(double min, double max)
{
    //qCDebug(KSANE_LOG) << "min,max(" << m_spinb->minimum() << " - " << m_spinb->maximum();
    //qCDebug(KSANE_LOG) << ") -> (" << min << " - " << max << ")" << std::endl;
    int imin = TO_FIX(min);
    int imax = TO_FIX(max);
    m_slider->setRange(imin, imax);
    //std::cout << "fixedmin=" << imin << " fixedmax=" << imax << std::endl;

    m_spinb->setRange(min, max);
}

void LabeledFSlider::setStep(double step)
{
    m_istep = TO_FIX(step);
    m_fstep = step;
    if (m_istep == 0) {
        m_istep = 1;
        m_fstep = TO_DOUBLE(m_istep);
    }
    m_slider->setSingleStep(m_istep);
    m_spinb->setSingleStep(m_fstep);

    int decimals = 0;
    double tmp_step = m_fstep;
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
    double newValue = value.toDouble(&ok);
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
    double value = TO_DOUBLE(ivalue);
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
        m_spinb->setValue(TO_DOUBLE(m_slider->value()));
    }
}

}  // NameSpace KSaneIface
