/* ============================================================
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
     * Create the labeled gamma widget.
     *
     * \param parent parent widget
     * \param text is the text describing the checkbox.
     * \param max is the maximum gamma-table-value
     */
    LabeledGamma(QWidget *parent, const QString &text, int max);
    
    LabeledGamma(QWidget *parent, KSaneOption *option, QColor color = Qt::black);
    ~LabeledGamma();

    void setColor(const QColor &color);
    
    int maxValue();

    bool getValues(int &brightness, int &contrast, int &gamma);

public Q_SLOTS:
    void setValues(const QVariant &values);

Q_SIGNALS:
    void valuesChanged(const QVariant &values);

private Q_SLOTS:
    void emitNewValues();
    
private:
    void initGamma(QString text, int max);
    
    LabeledSlider *m_brightSlider;
    LabeledSlider *m_contrastSlider;
    LabeledSlider *m_gammaSlider;

    int     m_brightness = 0;
    int     m_contrast = 0;
    int     m_gamma = 0;
    int     m_maxValue;
    
    GammaDisp  *m_gammaDisplay;
};

}  // NameSpace KSaneIface

#endif // LABELED_GAMMA_H
