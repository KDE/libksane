/*
 * SPDX-FileCopyrightText: 2019 Alexander Volkov <a.volkov@rusbitech.ru>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "hiderectitem.h"

#include <QPen>

namespace KSaneIface
{

HideRectItem::HideRectItem()
    : m_devicePixelRatio(1.0)
{
    setOpacity(0.4);
    setPen(Qt::NoPen);
    setBrush(Qt::black);
}

QRectF HideRectItem::rect() const
{
    QRectF r = QGraphicsRectItem::rect();
    return QRectF(r.topLeft() * m_devicePixelRatio, r.size() * m_devicePixelRatio);
}

void HideRectItem::setRect(const QRectF &rect)
{
    QGraphicsRectItem::setRect(QRectF(rect.topLeft() / m_devicePixelRatio, rect.size() / m_devicePixelRatio));
}

qreal HideRectItem::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

void HideRectItem::setDevicePixelRatio(qreal dpr)
{
    m_devicePixelRatio = dpr;
}

}  // NameSpace KSaneIface
