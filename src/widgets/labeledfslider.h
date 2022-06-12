/*
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LABELED_FSLIDER_H
#define LABELED_FSLIDER_H

#include "ksaneoptionwidget.h"

// Qt includes
#include <QSlider>
#include <QDoubleSpinBox>

/**
 *@author Kåre Särs
 */

namespace KSaneIface
{

/**
 * A combination of a label a slider and a spinbox.
 * The slider is connected to the spinbox so that they have the same value.
 */
class LabeledFSlider : public KSaneOptionWidget
{
    Q_OBJECT

public:

    /**
     * Create the slider.
     *
     * \param parent parent widget
     * \param text is the text describing the slider value. If the text
     *        contains a '&', a buddy for the slider will be created.
     * \param min minimum slider value
     * \param max maximum slider value
     * \param step is the step between values.
     */
    LabeledFSlider(QWidget *parent, const QString &text,
                   double min, double max, double step);

    LabeledFSlider(QWidget *parent, KSaneCore::Option *option);
    ~LabeledFSlider() override;

    /**
     * \return the slider value.
     */
    double value() const;
    double step() const;

public Q_SLOTS:

    /** Set the slider/spinbox value */
    void setValue(const QVariant &value);
    void setRange(double min, double max);
    void setStep(double step);
    /** Set the unit */
    void setSuffix(const QString &text);

private Q_SLOTS:

    /**
     * Sync the values and emit valueChanged()
     */
    // FIXME this has to be changes later
    void syncValues(int);
    void syncValues(double);
    void fixValue();

Q_SIGNALS:

    /**
     * Emit the slider value changes
     */
    void valueChanged(const QVariant &val);

private:
    void initFSlider(double minValue, double maxValue, double stepValue);

    QSlider        *m_slider;
    QDoubleSpinBox *m_spinb;
    double          m_fstep;
    int             m_istep;
};

}  // NameSpace KSaneIface

#endif // LABELED_SFSLIDER_H
