/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "labeledslider.h"

// Qt includes

#include <QLabel>
#include <QSlider>

// KDE includes

#include <kpluralhandlingspinbox.h>

namespace KSaneIface
{

LabeledSlider::LabeledSlider(QWidget *parent, const QString &ltext,
                             int min, int max, int ste)
    : KSaneOptionWidget(parent, ltext)
{
    m_step = ste;
    if (m_step == 0) {
        m_step = 1;
    }

    m_slider = new QSlider(this);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setMinimum(min);
    m_slider->setMaximum(max);
    m_slider->setSingleStep(m_step);

    m_spinb = new KPluralHandlingSpinBox(this);
    m_spinb->setMinimum(min);
    m_spinb->setMaximum(max);
    m_slider->setSingleStep(m_step);
    m_spinb->setValue(max);
    //m_spinb->setMinimumWidth(m_spinb->sizeHint().width()+35);
    m_spinb->setAlignment(Qt::AlignRight);
    m_spinb->setValue(min);

    m_spinb->setValue(min);
    m_label->setBuddy(m_spinb);

    connect(m_spinb, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabeledSlider::syncValues);
    connect(m_slider, &QSlider::valueChanged, this, &LabeledSlider::syncValues);
    connect(m_slider, &QSlider::sliderReleased, this, &LabeledSlider::fixValue);

    m_layout->addWidget(m_slider, 0, 2);
    m_layout->addWidget(m_spinb, 0, 1);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);

}

LabeledSlider::~LabeledSlider()
{
}

void LabeledSlider::setSuffix(const KLocalizedString &text)
{
    m_spinb->setSuffix(text);
}

void LabeledSlider::setValue(int value)
{
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
