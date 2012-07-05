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

// Local includes
#include "KSaneGammaDisp.h"
#include "KSaneGammaDisp.moc"

// Qt includes
#include <QPainter>

KSaneGammaDisp::KSaneGammaDisp(QWidget *parent, QVector<int> *tbl)
         : QWidget(parent)
{
    gam_tbl = tbl;
    gam_color.setRgb(0,0,0);
}

QSize KSaneGammaDisp::minimumSizeHint() const
{
    return QSize(75, 75);
}

QSize KSaneGammaDisp::sizeHint() const
{
    return QSize(75, 75);
}

void KSaneGammaDisp::setColor(const QColor &color)
{
    gam_color = color;
}

void KSaneGammaDisp::resizeEvent (QResizeEvent*)
{
    repaint();
}

void KSaneGammaDisp::paintEvent(QPaintEvent *)
{
/*    QMemArray<QRect> rects = event->region().rects();
    for (int i = 0; i < (int)rects.size(); i++) {
        bitBlt(this, rects[i].topLeft(), &pixmap, rects[i]);
    }*/
    QPointF p1, p2;
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(Qt::white));

    double xscale = (double)(size().width()-1)  / (double)gam_tbl->size();
    double yscale = (double)(size().height()-1) / (double)gam_tbl->size();

    painter.setPen(gam_color);
    for (int i=0; i<gam_tbl->size()-1; i++) {
        p1.setX(i*xscale);
        p1.setY(size().height()- 1 - (gam_tbl->at(i) * yscale));

        p2.setX((i+1)*xscale);
        p2.setY(size().height()- 1 - (gam_tbl->at(i+1) * yscale));

        painter.drawLine(p1, p2);
    }
}
