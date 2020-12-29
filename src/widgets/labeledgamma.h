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
