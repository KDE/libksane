/*
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GAMMA_DISP_H
#define GAMMA_DISP_H

// Qt includes

#include <QWidget>

/**
 *@author Kåre Särs
 *
 * This is the widget that displays the gamma table.
 */

namespace KSaneIface
{

class GammaDisp : public QWidget
{
    Q_OBJECT

public:

    /**
    * Create a gamma display.
    * \param parent parent widget
    * \param brightness the brightness value
    * \param contrast the contrast value
    * \param gamma the gamma value
    * \param maxValue the maximum value
    */
    GammaDisp(QWidget *parent, int *brightness, int *contrast, int *gamma, int maxValue);
    ~GammaDisp() override {}

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setColor(const QColor &color);

protected:

    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:

    int    *m_brightness;
    int    *m_contrast;
    int    *m_gamma;
    QColor  m_gammaColor;
    int     m_maxValue;
};

}  // NameSpace KSaneIface

#endif // GAMMA_DISP_H

