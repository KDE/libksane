/*
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LABELED_SLIDER_H
#define LABELED_SLIDER_H

#include "ksaneoptionwidget.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QSlider>
#include <QSpinBox>

/**
  *@author Kåre Särs
  */

namespace KSaneIface
{

/**
 * A combination of a label a slider and a spinbox.
 * The slider is connected to the spinbox so that they have the same value.
 */
class LabeledSlider : public KSaneOptionWidget
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
    LabeledSlider(QWidget *parent, const QString &text,
                  int min, int max, int st);

    LabeledSlider(QWidget *parent, KSaneCore::Option *option);
    ~LabeledSlider() override;

    int value() const;

public Q_SLOTS:

    /** Set the slider value */
    void setValue(const QVariant &val);
    void setRange(int min, int max);
    void setStep(int);

private Q_SLOTS:

    /** Sync the values and emit valueChanged() */
    void syncValues(int);
    /** Ensure step value is followed */
    void fixValue();

Q_SIGNALS:

    /**
     * Emit the slider value changes
     */
    void valueChanged(const QVariant &val);

private:
    int        m_step;
    void initSlider(int minValue, int maxValue, int stepValue);

    KLocalizedString m_spinboxFormat;
    QSlider   *m_slider;
    QSpinBox  *m_spinb;
};

}  // NameSpace KSaneIface

#endif // LABELED_SLIDER_H
