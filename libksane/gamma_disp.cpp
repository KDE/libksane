/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <QPainter>

// Local includes.

#include "gamma_disp.h"
#include "gamma_disp.moc"

namespace KSaneIface
{

GammaDisp::GammaDisp(QWidget *parent, QVector<int> *tbl)
         : QWidget(parent)
{
    gam_tbl = tbl;
    gam_color.setRgb(0,0,0);
}

QSize GammaDisp::minimumSizeHint() const
{
    return QSize(75, 75);
}

QSize GammaDisp::sizeHint() const
{
    return QSize(75, 75);
}

void GammaDisp::setColor(QColor color)
{
    gam_color = color;
}

void GammaDisp::resizeEvent (QResizeEvent*)
{
    repaint();
}

void GammaDisp::paintEvent(QPaintEvent *)
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
    for (int i=0; i<gam_tbl->size()-1; i++) 
    {
        p1.setX(i*xscale);
        p1.setY(size().height()- 1 - (gam_tbl->at(i) * yscale));

        p2.setX((i+1)*xscale);
        p2.setY(size().height()- 1 - (gam_tbl->at(i+1) * yscale));

        painter.drawLine(p1, p2);
    }
}

}  // NameSpace KSaneIface
