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

#ifndef LABELED_SFSLIDER_H
#define LABELED_SFSLIDER_H

// Qt includes.

#include <QFrame>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>

// Local includes.

#include "libksane_export.h"

/**
 *@author Kåre Särs
 */

namespace KSaneIface
{

/**
 * A combination of a label a slider and a spinbox.
 * The slider is connected to the spinbox so that they have the same value.
 */
class LIBKSANE_EXPORT LabeledFSlider : public QFrame
{
    Q_OBJECT

public:

   /**
    * Create the slider.
    *
    * \param parent parent widget
    * \param text is the text describing the the slider value. If the text
    *        contains a '&', a buddy for the slider will be created.
    * \param min minimum slider value
    * \param max maximum slider value
    * \param quant is the step between values.
    */
    LabeledFSlider(QWidget *parent, const QString& text,
                   float min, float max, float st);
    ~LabeledFSlider();

    void wigetSizeHints(int *lab_w, int *spi_w);
    void setColumnWidths(int lab_w, int spi_w);

   /**
    * \return the slider value.
    */
    float value() const {return (float)spinb->value();}
    float step() const {return fstep;}

public Q_SLOTS:
    
    /** Set the slider/spinbox value */
    void setValue(float);
    void setRange(float, float);
    void setStep(float);
    /** Set the unit */
    void setSuffix(const QString &text);

private Q_SLOTS:

    /**
     * Sync the values and emit valueChanged()
     */
    // FIXME this has to be changes later
    void syncValues(int);
    void syncValues(double);
    void fixValue(void);

Q_SIGNALS:

    /**
     * Emit the slider value changes
     */
    void valueChanged(float);

private:

    QLabel         *label;
    QSlider        *slider;
    QDoubleSpinBox *spinb;
    QGridLayout    *layout;
    float           fstep;
    int             istep;
};

}  // NameSpace KSaneIface

#endif // LABELED_SFSLIDER_H
