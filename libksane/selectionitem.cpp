/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2008-11-15
* Description :Selection QGraphicsItem for the image viewer.
*
* Copyright (C) 2008 by Kare Sars <kare dot sars at iki dot fi>
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
#include "selectionitem.h"
#include "selectionitem.moc"

namespace KSaneIface
{

static const int Margin = 4;

static const QPointF margin(Margin , Margin);

struct SelectionItem::Private
{
    QPen penBlack;
    QPen penWhite;
    QRectF rect;
    qreal maxX;
    qreal maxY;
};


SelectionItem::SelectionItem(QRectF rect) : QGraphicsItem(), d(new Private)
{
    d->maxX = rect.width();
    d->maxY = rect.height();
    d->rect = rect;
    d->penBlack.setColor(Qt::black);
    d->penBlack.setStyle(Qt::SolidLine);
    d->penWhite.setColor(Qt::white);
    d->penWhite.setStyle(Qt::DashLine);
}

SelectionItem::~SelectionItem()
{
    delete d;
}

void SelectionItem::setMaxRight(qreal maxX)
{
    d->maxX = maxX;
}

void SelectionItem::setMaxBottom(qreal maxY)
{
    d->maxY = maxY;
}

SelectionItem::Intersects SelectionItem::intersects(QPointF point)
{
    if (point.x() < (d->rect.left()-Margin))   return None;
    if (point.x() > (d->rect.right()+Margin))  return None;
    if (point.y() < (d->rect.top()-Margin))    return None;
    if (point.y() > (d->rect.bottom()+Margin)) return None;

    if (point.x() < (d->rect.left()+Margin)) {
        if (point.y() < (d->rect.top()+Margin))    return TopLeft;
        if (point.y() > (d->rect.bottom()-Margin)) return BottomLeft;
        return Left;
    }

    if (point.x() > (d->rect.right()-Margin)) {
        if (point.y() < (d->rect.top()+Margin))    return TopRight;
        if (point.y() > (d->rect.bottom()-Margin)) return BottomRight;
        return Right;
    }

    if (point.y() < (d->rect.top()+Margin))    return Top;
    if (point.y() > (d->rect.bottom()-Margin)) return Bottom;

    return Move;
}

void SelectionItem::setRect(QRectF rect)
{
    prepareGeometryChange();
    d->rect = rect;
    d->rect = d->rect.normalized();
    if (d->rect.top() < 0) d->rect.setTop(0);
    if (d->rect.left() < 0) d->rect.setLeft(0);
    if (d->rect.right() > d->maxX) d->rect.setRight(d->maxX);
    if (d->rect.bottom() > d->maxY) d->rect.setBottom(d->maxY);
}

QPointF SelectionItem::fixTranslation(QPointF dp)
{
    if ((d->rect.left()   + dp.x()) < 0) dp.setX(-d->rect.left());
    if ((d->rect.top()    + dp.y()) < 0) dp.setY(-d->rect.top());
    if ((d->rect.right()  + dp.x()) > d->maxX) dp.setX(d->maxX - d->rect.right());
    if ((d->rect.bottom() + dp.y()) > d->maxY) dp.setY(d->maxY - d->rect.bottom());
    return dp;
}

QRectF SelectionItem::rect()
{
    return d->rect;
}

QRectF SelectionItem::boundingRect() const
{
    return QRectF(d->rect.topLeft()-margin, d->rect.bottomRight()+margin);
}

void SelectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{

    painter->setPen(d->penBlack);
    painter->drawRect(d->rect);

    painter->setPen(d->penWhite);
    painter->drawRect(d->rect);
}


}  // NameSpace KSaneIface
