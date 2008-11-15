#ifndef SIDELINEITEM_H
#define SIDELINEITEM_H

#include <QGraphicsPixmapItem>
#include <QPainter>

namespace KSaneIface
{

class SelectionItem : public QGraphicsItem
{
    public:
        typedef enum
        {
            None,
            Top,
            TopRight,
            Right,
            BottomRight,
            Bottom,
            BottomLeft,
            Left,
            TopLeft,
            Move
        } Intersects;
        
        explicit SelectionItem(QRectF rect);
        ~SelectionItem();

        void setMaxRight(qreal maxRight);
        void setMaxBottom(qreal maxBottom);

        Intersects intersects(QPointF point);

        void setRect(QRectF rect);
        QPointF fixTranslation(QPointF dp);
        QRectF rect();

        // Graphics Item methods
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    private:
        struct Private;
        Private * const d;
};

}  // NameSpace KSaneIface

#endif

