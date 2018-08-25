/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2011 by Kare Sars <kare.sars@iki .fi>
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

#ifndef LABELED_GAMMA_H
#define LABELED_GAMMA_H

// Local includes
#include "labeledslider.h"
#include "gammadisp.h"

/**
  *@author Kåre Särs
  */

namespace KSaneIface
{

/**
 * A wrapper for a checkbox
 */
class LabeledGamma : public KSaneOptionWidget
{
    Q_OBJECT

public:

    /**
     * Create the checkbox.
     *
     * \param parent parent widget
     * \param text is the text describing the checkbox.
     * \param elements is the number of elements in the gamma table
     * \param max is the maximum gamma-table-value
     */
    LabeledGamma(QWidget *parent, const QString &text, int elements, int max);
    ~LabeledGamma();

    void setColor(const QColor &color);
    void setSize(int size);
    const QVector<int> &gammaTablePtr();
    int size();

    int maxValue();

    bool getValues(int &bri, int &con, int &gam);

public Q_SLOTS:
    void setValues(int bri, int con, int gam);
    void setValues(const QString &values);

private Q_SLOTS:
    void calculateGT();

Q_SIGNALS:

    void gammaChanged(int bri, int con, int gam);
    void gammaTableChanged(const QVector<int> &gammaTable);

private:

    LabeledSlider *m_brightSlider;
    LabeledSlider *m_contrastSlider;
    LabeledSlider *m_gammaSlider;

    QVector<int>   m_gammaTable;
    double         m_maxValue;

    GammaDisp     *m_gammaDisplay;
};

}  // NameSpace KSaneIface

#endif // LABELED_GAMMA_H
