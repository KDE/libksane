/* ============================================================
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

#include <cmath>

namespace KSaneIface
{

GammaDisp::GammaDisp(QWidget *parent, int *brightness, int *contrast, int *gamma, int maxValue)
: QWidget(parent)
, m_brightness(brightness)
, m_contrast(contrast)
, m_gamma(gamma)
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

    const int xResolution = 100;
    double max = static_cast<double>(m_maxValue);
    double xscale = static_cast<double>(size().width() - 1)  / static_cast<double>(xResolution);
    double yscale = static_cast<double>(size().height() - 1) / max;

    painter.setPen(m_gammaColor);

    double gamma    = 100.0 / *m_gamma;
    double contrast = (200.0 / (100.0 - *m_contrast)) - 1;
    double halfMax  = max / 2.0;
    double brightness  = (*m_brightness / halfMax) * max;

    double xPrevious = 0;
    double xNext = 0;
    
    // gamma is zero for first one, start with contrast
    xPrevious = (contrast * (xPrevious - halfMax)) + halfMax;
    // apply brightness + rounding
    xPrevious += brightness + 0.5;
    // ensure correct value
    if (xPrevious > max) {
        xPrevious = max;
    }
    if (xPrevious < 0) {
        xPrevious = 0;
    }

    for (int i = 0; i < xResolution - 1; i++) {
        xNext = std::pow(static_cast<double>(i+1) / xResolution, gamma) * max;
        // apply contrast
        xNext = (contrast * (xNext - halfMax)) + halfMax;
        // apply brightness + rounding
        xNext += brightness + 0.5;
        
        // ensure correct value
        if (xNext > max) {
            xNext = max;
        }
        if (xNext < 0) {
            xNext = 0;
        }
        
        p1.setX(i * xscale);
        p1.setY(size().height() - 1 - (xPrevious * yscale));

        p2.setX((i + 1)*xscale);
        p2.setY(size().height() - 1 - (xNext * yscale));

        painter.drawLine(p1, p2);
        xPrevious = xNext;
    }
}

}  // NameSpace KSaneIface
