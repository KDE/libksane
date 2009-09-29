/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

#ifndef LABELED_SLIDER_H
#define LABELED_SLIDER_H

// Qt includes.

#include <QFrame>
#include <QSlider>
#include <QGridLayout>

/** Remove #if tags when KDE 4.2 becomes unpopular**/
#include <kdeversion.h>

/**
  *@author Kåre Särs
  */

class KIntSpinBox;
class KLocalizedString;
class QLabel;

namespace KSaneIface
{

/**
 * A combination of a label a slider and a spinbox.
 * The slider is connected to the spinbox so that they have the same value.
 */
class LabeledSlider : public QFrame
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
    * \param quant is the step between values.
    */
    LabeledSlider(QWidget *parent, const QString& text,
                int min, int max, int st);
    ~LabeledSlider();

    /**
    * Set the label
    */
    void setLabelText(const QString &text);
    
    void widgetSizeHints(int *lab_w, int *spi_w);
    void setColumnWidths(int lab_w, int spi_w);
    int value() const { return( m_slider->value()); }

public Q_SLOTS:

  /** Set the slider value */
    void setValue(int);
    void setRange(int min, int max);
    void setStep(int);
    /** Set the unit */
    void setSuffix(const KLocalizedString &text);
    void setSuffix1(const QString &text); /** Remove this compatibility hack when KDE 4.2 becomes unpopular **/

private Q_SLOTS:

    /** Sync the values and emit valueChanged() */
    void syncValues(int);
    /** Ensure step value is followed */
    void fixValue();

Q_SIGNALS:

    /**
     * Emit the slider value changes
     */
    void valueChanged(int);

private:

    QLabel      *m_label;
    QSlider     *m_slider;
    KIntSpinBox *m_spinb;
    int          m_step;
    QGridLayout *m_layout;
};

}  // NameSpace KSaneIface

#endif // LABELED_SLIDER_H
