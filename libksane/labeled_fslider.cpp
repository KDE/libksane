/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#define FLOAT_MULTIP 32768.0
#define TO_FLOAT(v) ((float)v / FLOAT_MULTIP)
#define TO_FIX(v) ((int)(v * FLOAT_MULTIP))

// Local includes.

#include "labeled_fslider.h"
#include "labeled_fslider.moc"

namespace KSaneIface
{

LabeledFSlider::LabeledFSlider(QWidget *parent, const QString& ltext,
                               float min, float max, float step)
              : QFrame(parent)
{
    layout = new QGridLayout(this);
    layout->setSpacing(3);
    layout->setMargin(0);
    label = new QLabel(ltext, this);

    int imin = TO_FIX(min);
    int imax = TO_FIX(max);
    istep = TO_FIX(step);
    fstep = step;
    if (istep == 0) {
        istep = 1;
        fstep = TO_FLOAT(istep);
    }

    //std::cout << "min=" << min << ", max=" << max << ", fstep="<<fstep<<std::endl;
    //std::cout << "imin=" << imin << ", imax=" << imax << ", istep="<<istep<<std::endl;
    slider = new QSlider(this);
    slider->setOrientation(Qt::Horizontal);
    slider->setMinimum(imin);
    slider->setMaximum(imax);
    slider->setSingleStep(istep);
    slider->setValue(imin);

    spinb = new QDoubleSpinBox(this);
    spinb->setMinimum(min);
    spinb->setMaximum(max);
    spinb->setSingleStep(fstep);
    int decimals=0;
    float tmp_step = fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) break;
    }
    spinb->setDecimals(decimals);
    spinb->setValue(max);
    spinb->setMinimumWidth(spinb->sizeHint().width()+35);
    spinb->setAlignment(Qt::AlignRight);
    spinb->setValue(min);

    label->setBuddy(slider);

    connect(spinb, SIGNAL(valueChanged(double)),
            this, SLOT(syncValues(double)));

    connect(slider, SIGNAL(valueChanged(int)),
            this, SLOT(syncValues(int)));

    connect(slider, SIGNAL(sliderReleased()),
            this, SLOT(fixValue()));

    layout->addWidget(label, 0, 0);
    layout->addWidget(slider, 0,1);
    layout->addWidget(spinb, 0, 2);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 50);
    layout->setColumnStretch(2, 0);
    layout->activate();
}

LabeledFSlider::~LabeledFSlider()
{
}

void LabeledFSlider::wigetSizeHints(int *lab_w, int *spi_w)
{
    if (lab_w != 0) *lab_w = label->sizeHint().width();
    if (spi_w != 0) *spi_w = spinb->sizeHint().width();
}

void LabeledFSlider::setColumnWidths(int lab_w, int spi_w)
{
    layout->setColumnMinimumWidth(0, lab_w);
    layout->setColumnMinimumWidth(2, spi_w);
}

void LabeledFSlider::setSuffix(const QString &text)
{
    spinb->setSuffix(text);
}

void LabeledFSlider::setRange(float min, float max)
{
    //kDebug() << "min,max(" << spinb->minimum() << " - " << spinb->maximum();
    //kDebug() << ") -> (" << min << " - " << max << ")" << std::endl;
    int imin=TO_FIX(min);
    int imax=TO_FIX(max);
    slider->setRange(imin, imax);
    //std::cout << "fixedmin=" << imin << " fixedmax=" << imax << std::endl;

    spinb->setRange(min, max);
}

void LabeledFSlider::setStep(float step)
{
    istep = TO_FIX(step);
    fstep = step;
    if (istep == 0) {
        istep = 1;
        fstep = TO_FLOAT(istep);
    }
    slider->setSingleStep(istep);
    spinb->setSingleStep(fstep);

    int decimals=0;
    float tmp_step = fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) break;
    }
    spinb->setDecimals(decimals);
}

void LabeledFSlider::setValue(float value)
{
    int ivalue = TO_FIX(value);

    if (((value - spinb->value()) > fstep) || ((spinb->value() - value) > fstep)) {
        spinb->setValue(value);
    }
    else if (ivalue != slider->value()) slider->setValue(ivalue);
}

void LabeledFSlider::syncValues(int ivalue)
{
    double value = TO_FLOAT(ivalue);
    if (((value - spinb->value()) > fstep) || ((spinb->value() - value) > fstep)) {
        spinb->setValue(value);
    }
    else if (ivalue != slider->value())slider->setValue(ivalue);
    else emit valueChanged(value);
}

void LabeledFSlider::syncValues(double value)
{
    int ivalue = TO_FIX(value);
    if (((value - spinb->value()) > fstep) || ((spinb->value() - value) > fstep)) {
        spinb->setValue(value);
    }
    else if (ivalue != slider->value())slider->setValue(ivalue);
    else emit valueChanged((float)value);
}

void LabeledFSlider::fixValue()
{
    //ensure that the value step is followed also for the slider
    int rest = (slider->value() - slider->minimum())%istep;

    if (rest != 0) {
        if (rest > (istep/2)) slider->setValue(slider->value()+(istep-rest));
        else                  slider->setValue(slider->value()-rest);
        spinb->setValue(TO_FLOAT(slider->value()));
    }
}

}  // NameSpace KSaneIface
