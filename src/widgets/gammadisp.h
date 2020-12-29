/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

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
    * \param tbl the gamma table
    * \param maxValue the maximum value
    */
    GammaDisp(QWidget *parent, QVector<int> *tbl, int maxValue);
    ~GammaDisp() {}

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setColor(const QColor &color);

protected:

    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:

    QVector<int> *m_gammaTable;
    QColor        m_gammaColor;
    int           m_maxValue;
};

}  // NameSpace KSaneIface

#endif // GAMMA_DISP_H

