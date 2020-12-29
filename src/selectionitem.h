/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2008-11-15
* Description :Selection QGraphicsItem for the image viewer.
*
* SPDX-FileCopyrightText: 2008 Kare Sars <kare dot sars at iki dot fi>
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */
#ifndef SELECTIONITEM_H
#define SELECTIONITEM_H

#include <QGraphicsPixmapItem>
#include <QPainter>

namespace KSaneIface
{

class SelectionItem : public QGraphicsItem
{
public:
    typedef enum {
        None,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        TopLeft,
        Move,
        AddRemove
    } Intersects;

    explicit SelectionItem(const QRectF &rect);
    ~SelectionItem();

    void setMaxRight(qreal maxRight);
    void setMaxBottom(qreal maxBottom);
    void setSaved(bool isSaved);

    Intersects intersects(const QPointF &point);

    void saveZoom(qreal zoom);

    void setRect(const QRectF &rect);
    QPointF fixTranslation( QPointF dp);
    QRectF rect();

    qreal devicePixelRatio() const;
    void setDevicePixelRatio(qreal dpr);

    void setAddButtonEnabled(bool enabled);

public:
    // Graphics Item methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    struct Private;
    Private *const d;
};

}  // NameSpace KSaneIface

#endif

