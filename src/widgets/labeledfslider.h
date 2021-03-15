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
     * \param st is the step between values.
     */
    LabeledFSlider(QWidget *parent, const QString &text,
                   float min, float max, float step);

    LabeledFSlider(QWidget *parent, KSaneOption *option);
    ~LabeledFSlider();

    /**
     * \return the slider value.
     */
    float value() const;
    float step() const;

public Q_SLOTS:

    /** Set the slider/spinbox value */
    void setValue(const QVariant &value);
    void setRange(float min, float max);
    void setStep(float step);
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
    void initFSlider(float minValueF, float maxValueF, float stepValueF);

    QSlider        *m_slider;
    QDoubleSpinBox *m_spinb;
    float           m_fstep;
    int             m_istep;
};

}  // NameSpace KSaneIface

#endif // LABELED_SFSLIDER_H
