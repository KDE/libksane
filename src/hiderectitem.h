/*
 * SPDX-FileCopyrightText: 2019 Alexander Volkov <a.volkov@rusbitech.ru>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef HIDERECTITEM_H
#define HIDERECTITEM_H

#include <QGraphicsRectItem>

namespace KSaneIface
{

/**
 * QGraphicsItem for hiding rects in the image viewer.
 */
class HideRectItem : public QGraphicsRectItem
{
public:
    HideRectItem();

    QRectF rect() const;
    void setRect(const QRectF &rect);

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal dpr);

private:
    qreal m_devicePixelRatio;
};

}  // NameSpace KSaneIface

#endif // HIDERECTITEM_H
