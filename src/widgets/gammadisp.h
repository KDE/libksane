/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare.sars@iki .fi>
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

