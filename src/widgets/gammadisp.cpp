/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

// Local includes
#include "gammadisp.h"

// Qt includes
#include <QPainter>

namespace KSaneIface
{

GammaDisp::GammaDisp(QWidget *parent, QVector<int> *tbl, int maxValue)
: QWidget(parent)
, m_gammaTable(tbl)
, m_gammaColor(QColor::fromRgb(0,0,0))
, m_maxValue(maxValue)
{}

QSize GammaDisp::minimumSizeHint() const
{
    return QSize(75, 75);
}

QSize GammaDisp::sizeHint() const
{
    return QSize(75, 75);
}

void GammaDisp::setColor(const QColor &color)
{
    m_gammaColor = color;
}

void GammaDisp::resizeEvent(QResizeEvent *)
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

    double xscale = (double)(size().width() - 1)  / (double)m_gammaTable->size();
    double yscale = (double)(size().height() - 1) / (double)m_maxValue;

    painter.setPen(m_gammaColor);
    for (int i = 0; i < m_gammaTable->size() - 1; i++) {
        p1.setX(i * xscale);
        p1.setY(size().height() - 1 - (m_gammaTable->at(i) * yscale));

        p2.setX((i + 1)*xscale);
        p2.setY(size().height() - 1 - (m_gammaTable->at(i + 1) * yscale));

        painter.drawLine(p1, p2);
    }
}

}  // NameSpace KSaneIface
