/*
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "labeledslider.h"
// Qt includes

#include <QLabel>
#include <QSlider>

// KDE includes

#include <KLocalization>
#include <KSaneCore/Option>

namespace KSaneIface
{
LabeledSlider::LabeledSlider(QWidget *parent, const QString &ltext,
                             int min, int max, int ste)
    : KSaneOptionWidget(parent, ltext)
{
    initSlider(min, max, ste);
}

LabeledSlider::LabeledSlider(QWidget *parent, KSaneCore::Option *option)
    : KSaneOptionWidget(parent, option)
{
    int maxValue = option->maximumValue().toInt();
    int minValue = option->minimumValue().toInt();
    int stepValue = option->stepValue().toInt();

    initSlider(minValue, maxValue, stepValue);

    KSaneCore::Option::OptionUnit unit = option->valueUnit();
    switch (unit) {
    case KSaneCore::Option::UnitPixel:
        m_spinboxFormat = ki18ncp("@label:spinbox", "%v pixel", "%v pixels");
        break;
    case KSaneCore::Option::UnitBit:
        m_spinboxFormat = ki18ncp("@label:spinbox", "%v bit", "%v bits");
        break;
    case KSaneCore::Option::UnitMilliMeter:
        m_spinboxFormat = QLocale().measurementSystem() == QLocale::MetricSystem
            ? ki18nc("@label:spinbox, milimeters", "%v mm")
            : ki18nc("@label:spinbox, milimeters", "%v in");
        break;
    case KSaneCore::Option::UnitDPI:
        m_spinboxFormat = ki18nc("@label:spinbox, dots per inch", "%v DPI");
        break;
    case KSaneCore::Option::UnitPercent:
        m_spinboxFormat = ki18nc("@label:spinbox, percentage", "%v%");
        break;
    case KSaneCore::Option::UnitMicroSecond:
        m_spinboxFormat = ki18nc("@label:spinbox, microsecond", "%v µs");
        break;
    case KSaneCore::Option::UnitSecond:
        m_spinboxFormat = ki18nc("@label:spinbox, second", "%v s");
        break;
    default:
        m_spinboxFormat = ki18n("%v");
        break;
    }

    setLabelText(option->title());
    setToolTip(option->description());
    connect(this, &LabeledSlider::valueChanged, option, &KSaneCore::Option::setValue);
    connect(option, &KSaneCore::Option::valueChanged, this, &LabeledSlider::setValue);
    int value = option->value().toInt();
    setValue(value);
}

LabeledSlider::~LabeledSlider()
{
}

void LabeledSlider::initSlider(int minValue, int maxValue, int stepValue)
{
    m_step = stepValue;
    if (m_step == 0) {
        m_step = 1;
    }

    m_slider = new QSlider(this);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setMinimum(minValue);
    m_slider->setMaximum(maxValue);
    m_slider->setSingleStep(m_step);

    m_spinb = new QSpinBox(this);
    m_spinb->setMinimum(minValue);
    m_spinb->setMaximum(maxValue);
    m_slider->setSingleStep(m_step);
    m_spinb->setValue(maxValue);
    m_spinb->setAlignment(Qt::AlignRight);
    m_spinb->setValue(minValue);
    KLocalization::setupSpinBoxFormatString(m_spinb, m_spinboxFormat);

    m_label->setBuddy(m_spinb);

    connect(m_spinb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabeledSlider::syncValues);
    connect(m_slider, &QSlider::valueChanged, this, &LabeledSlider::syncValues);
    connect(m_slider, &QSlider::sliderReleased, this, &LabeledSlider::fixValue);

    m_layout->addWidget(m_slider, 0, 2);
    m_layout->addWidget(m_spinb, 0, 1);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);
}

void LabeledSlider::setValue(const QVariant &val)
{
    bool ok;
    int value = val.toInt(&ok);
    if (!ok) {
        return;
    }
    if (value != m_slider->value()) {
        m_slider->setValue(value);
    } else if (value != m_spinb->value()) {
        m_spinb->setValue(value);
    }
}

void LabeledSlider::setRange(int min, int max)
{
    //std::cout << "min=" << min << ", max=" << max << std::endl;
    m_slider->setRange(min, max);
    m_spinb->setRange(min, max);
}

void LabeledSlider::setStep(int st)
{
    m_step = st;
    if (m_step == 0) {
        m_step = 1;
    }
    m_slider->setSingleStep(m_step);
    m_spinb->setSingleStep(m_step);
}

void LabeledSlider::syncValues(int value)
{
    if (value != m_spinb->value()) {
        m_spinb->setValue(value);
    } else if (value != m_slider->value()) {
        //ensure that the value m_step is followed also for the m_slider
        if ((value - m_slider->minimum()) % m_step != 0) {
            if (value > m_spinb->value()) {
                m_slider->setValue(m_slider->value() + (m_step - (value - m_spinb->value())));
            } else {
                m_slider->setValue(m_slider->value() - (m_step - (m_spinb->value() - value)));
            }
            // this function will be reentered with the signal fom the m_slider
        } else {
            m_slider->setValue(value);
        }
    } else {
        Q_EMIT valueChanged(value);
    }
}

void LabeledSlider::fixValue()
{
    //ensure that the value m_step is followed also for the m_slider
    int rest = (m_slider->value() - m_slider->minimum()) % m_step;

    if (rest != 0) {
        if (rest > (m_step / 2)) {
            m_slider->setValue(m_slider->value() + (m_step - rest));
        } else {
            m_slider->setValue(m_slider->value() - rest);
        }
        m_spinb->setValue(m_slider->value());
    }
}
int LabeledSlider::value() const
{
    return (m_slider->value());
}

}  // NameSpace KSaneIface

#include "moc_labeledslider.cpp"
