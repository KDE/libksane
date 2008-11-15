/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2008-11-15
* Description : Preview image viewer that can handle a selection.
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

#include "ksane_viewer.h"
#include "ksane_viewer.moc"
#include "selectionitem.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QScrollBar>
#include <QAction>
#include <KIcon>
#include <KLocale>

#include <KDebug>

namespace KSaneIface
{

struct KSaneViewer::Private
{
    QGraphicsScene      *scene;
    QGraphicsPixmapItem *pixmapItem;
    SelectionItem       *selection;
    QImage              *img;
    
    SelectionItem::Intersects change;

    QPointF lastSPoint;
    int m_left_last_x;
    int m_left_last_y;

    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomSelAction;
    QAction *zoom2FitAction;
};

KSaneViewer::KSaneViewer(QWidget *parent) : QGraphicsView(parent), d(new Private)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setMouseTracking(true);
    
    // Init the scene
    d->scene = new QGraphicsScene;
    setScene(d->scene);
    d->pixmapItem = new QGraphicsPixmapItem;

    d->selection = new SelectionItem(QRectF());
    d->selection->setZValue(10);

    d->scene->addItem(d->pixmapItem);
    d->scene->addItem(d->selection);
    d->scene->setBackgroundBrush(Qt::gray);

    d->change = SelectionItem::None;

    // create context menu
    d->zoomInAction = new QAction(KIcon("zoom-in"), i18n("Zoom In"), this);
    connect(d->zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
    
    d->zoomOutAction = new QAction(KIcon("zoom-out"), i18n("Zoom Out"), this);
    connect(d->zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));
    
    d->zoomSelAction = new QAction(KIcon("zoom-fit-best"), i18n("Zoom to Selection"), this);
    connect(d->zoomSelAction, SIGNAL(triggered()), this, SLOT(zoomSel()));
    
    d->zoom2FitAction = new QAction(KIcon("document-preview"), i18n("Zoom to Fit"), this);
    connect(d->zoom2FitAction, SIGNAL(triggered()), this, SLOT(zoom2Fit()));
    
    addAction(d->zoomInAction);
    addAction(d->zoomOutAction);
    addAction(d->zoomSelAction);
    addAction(d->zoom2FitAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

// ------------------------------------------------------------------------
KSaneViewer::~KSaneViewer()
{
    delete d;
}

// ------------------------------------------------------------------------
void KSaneViewer::setQImage(QImage *img)
{
    if (img == 0) return;
    d->pixmapItem->setPixmap(QPixmap::fromImage(*img));
    d->pixmapItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    d->scene->setSceneRect(0, 0, img->width(), img->height());
    d->pixmapItem->setZValue(0);
    d->selection->setMaxRight(img->width());
    d->selection->setMaxBottom(img->height());
    d->selection->setRect(d->scene->sceneRect());
    d->selection->setVisible(false);
    d->img = img;
}

// ------------------------------------------------------------------------
void KSaneViewer::updateImage()
{
    d->pixmapItem->setPixmap(QPixmap::fromImage(*d->img));
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomIn()
{
    scale(1.5, 1.5);
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomOut()
{
    scale(1.0 / 1.5, 1.0 / 1.5);
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomSel()
{
    fitInView(d->selection->boundingRect() , Qt::KeepAspectRatio);
}

// ------------------------------------------------------------------------
void KSaneViewer::zoom2Fit()
{
    fitInView(d->pixmapItem->boundingRect(), Qt::KeepAspectRatio);
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLX(float percentage)
{
    QRectF rect = d->selection->rect();
    rect.setLeft(percentage * d->pixmapItem->pixmap().width());
    d->selection->setRect(rect);
    if (rect.left() > 0.001) {
        d->selection->setVisible(true);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLY(float percentage)
{
    QRectF rect = d->selection->rect();
    rect.setTop(percentage * d->pixmapItem->pixmap().height());
    d->selection->setRect(rect);
    if (rect.top() > 0.001) {
        d->selection->setVisible(true);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRX(float percentage)
{
    QRectF rect = d->selection->rect();
    rect.setRight(percentage * d->pixmapItem->pixmap().width());
    d->selection->setRect(rect);
    if (d->pixmapItem->pixmap().width() - rect.right() > 0.001) {
        d->selection->setVisible(true);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRY(float percentage)
{
    QRectF rect = d->selection->rect();
    rect.setBottom(percentage * d->pixmapItem->pixmap().height());
    d->selection->setRect(rect);
    if (d->pixmapItem->pixmap().height() - rect.bottom() > 0.001) {
        d->selection->setVisible(true);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setSelection(float percent_tlx, float percent_tly, float percent_brx, float percent_bry)
{
    QRectF rect;
    rect.setCoords(percent_tlx * d->pixmapItem->pixmap().width(),
                    percent_tly * d->pixmapItem->pixmap().height(),
                    percent_brx * d->pixmapItem->pixmap().width(),
                    percent_bry * d->pixmapItem->pixmap().height());

    d->selection->setRect(rect);
    d->selection->setVisible(true);
    kDebug() << rect;
}

// ------------------------------------------------------------------------
void KSaneViewer::clearSelection()
{
    d->selection->setRect(d->scene->sceneRect());
    d->selection->setVisible(false);
}

// ------------------------------------------------------------------------
void KSaneViewer::wheelEvent(QWheelEvent *e)
{
    if(e->modifiers() == Qt::ControlModifier) {
        if(e->delta() > 0) {
            zoomIn();
        } else {
            zoomOut();
        }
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        d->m_left_last_x = e->x();
        d->m_left_last_y = e->y();
        QPointF scenePoint = mapToScene(e->pos());
        d->lastSPoint = scenePoint;
        if (e->modifiers() != Qt::ControlModifier) {
            if (!d->selection->isVisible()) {
                d->selection->setVisible(true);
                d->selection->setRect(QRectF(scenePoint, QSizeF(0,0)));
                d->change = SelectionItem::BottomRight;
            }
            else if (!d->selection->contains(scenePoint)) {
                d->selection->setRect(QRectF(scenePoint, QSizeF(0,0)));
                d->change = SelectionItem::BottomRight;
            }
        }
    }
    QGraphicsView::mousePressEvent(e);
}

// ------------------------------------------------------------------------
void KSaneViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if ((d->selection->rect().width() < 0.001) ||
            (d->selection->rect().height() < 0.001))
        {
            emit newSelection(0,0, d->scene->sceneRect().width(), d->scene->sceneRect().height());
            d->selection->setRect(d->scene->sceneRect());
            d->selection->setVisible(false);
        }
    }
    
    if ((e->modifiers() != Qt::ControlModifier) &&
        (d->selection->isVisible()) &&
        (d->pixmapItem->pixmap().width() != 0) &&
        (d->pixmapItem->pixmap().height() != 0))
    {
        float tlx = d->selection->rect().left()   / d->pixmapItem->pixmap().width();
        float tly = d->selection->rect().top()    / d->pixmapItem->pixmap().height();
        float brx = d->selection->rect().right()  / d->pixmapItem->pixmap().width();
        float bry = d->selection->rect().bottom() / d->pixmapItem->pixmap().height();
        
        emit newSelection(tlx, tly, brx, bry);
    }

    QGraphicsView::mouseReleaseEvent(e);
}

// ------------------------------------------------------------------------
void KSaneViewer::mouseMoveEvent(QMouseEvent *e)
{
    QPointF scenePoint = mapToScene(e->pos());
    
    if (e->buttons()&Qt::LeftButton)
    {
        if (e->modifiers() == Qt::ControlModifier)
        {
            int dx = e->x() - d->m_left_last_x;
            int dy = e->y() - d->m_left_last_y;
            verticalScrollBar()->setValue(verticalScrollBar()->value()-dy);
            horizontalScrollBar()->setValue(horizontalScrollBar()->value()-dx);
            d->m_left_last_x = e->x();
            d->m_left_last_y = e->y();
        }
        else {
            ensureVisible(QRectF(scenePoint, QSizeF(0,0)), 1, 1);
            QRectF rect = d->selection->rect();
            switch (d->change)
            {
                case SelectionItem::None:
                    // should not be here :)
                    break;
                case SelectionItem::Top:
                    if (scenePoint.y() < rect.bottom()) rect.setTop(scenePoint.y());
                    else {
                        d->change = SelectionItem::Bottom;
                        rect.setBottom(scenePoint.y());
                    }
                    break;
                case SelectionItem::TopRight:
                    if (scenePoint.x() > rect.left()) rect.setRight(scenePoint.x());
                    else {
                        rect.setLeft(scenePoint.x());
                        d->change = SelectionItem::TopLeft;
                    }
                    if (scenePoint.y() < rect.bottom()) rect.setTop(scenePoint.y());
                    else {
                        rect.setBottom(scenePoint.y());
                        d->change = SelectionItem::BottomLeft;
                    } // FIXME arrow
                    break;
                case SelectionItem::Right:
                    if (scenePoint.x() > rect.left()) rect.setRight(scenePoint.x());
                    else {
                        rect.setLeft(scenePoint.x());
                        d->change = SelectionItem::Left;
                    }
                    break;
                case SelectionItem::BottomRight:
                    if (scenePoint.x() > rect.left()) rect.setRight(scenePoint.x());
                    else {
                        rect.setLeft(scenePoint.x());
                        d->change = SelectionItem::BottomLeft;
                    }
                    if (scenePoint.y() > rect.top())  rect.setBottom(scenePoint.y());
                    else {
                        rect.setTop(scenePoint.y());
                        d->change = SelectionItem::TopRight;
                    } // FIXME arrow
                    break;
                case SelectionItem::Bottom:
                    if (scenePoint.y() > rect.top()) rect.setBottom(scenePoint.y());
                    else {
                        d->change = SelectionItem::Top;
                        rect.setTop(scenePoint.y());
                    }
                    break;
                case SelectionItem::BottomLeft:
                    if (scenePoint.x() < rect.right()) rect.setLeft(scenePoint.x());
                    else {
                        rect.setRight(scenePoint.x());
                        d->change = SelectionItem::BottomRight;
                    }
                    if (scenePoint.y() > rect.top()) rect.setBottom(scenePoint.y());
                    else {
                        rect.setTop(scenePoint.y());
                        d->change = SelectionItem::TopLeft;
                    } // FIXME arrow
                    break;
                case SelectionItem::Left:
                    if (scenePoint.x() < rect.right()) rect.setLeft(scenePoint.x());
                    else {
                        rect.setRight(scenePoint.x());
                        d->change = SelectionItem::Right;
                    }
                    break;
                case SelectionItem::TopLeft:
                    if (scenePoint.x() < rect.right())  rect.setLeft(scenePoint.x());
                    else {
                        rect.setRight(scenePoint.x());
                        d->change = SelectionItem::TopRight;
                    }
                    if (scenePoint.y() < rect.bottom()) rect.setTop(scenePoint.y());
                    else {
                        rect.setBottom(scenePoint.y());
                        d->change = SelectionItem::BottomLeft;
                    }// FIXME arrow
                    break;
                case SelectionItem::Move:
                    rect.translate(d->selection->fixTranslation(scenePoint-d->lastSPoint));
                    break;
            }
            d->selection->setRect(rect);
        }
    }
    else if (d->selection->isVisible()) {
        d->change = d->selection->intersects(scenePoint);

        switch (d->change)
        {
            case SelectionItem::None:
                setCursor(Qt::CrossCursor);
                break;
            case SelectionItem::Top:
                setCursor(Qt::SizeVerCursor);
                break;
            case SelectionItem::TopRight:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case SelectionItem::Right:
                setCursor(Qt::SizeHorCursor);
                break;
            case SelectionItem::BottomRight:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case SelectionItem::Bottom:
                setCursor(Qt::SizeVerCursor);
                break;
            case SelectionItem::BottomLeft:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case SelectionItem::Left:
                setCursor(Qt::SizeHorCursor);
                break;
            case SelectionItem::TopLeft:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case SelectionItem::Move:
                setCursor(Qt::SizeAllCursor);
                break;
        }
    }
    d->lastSPoint = scenePoint;
    QGraphicsView::mouseMoveEvent(e);
}


}  // NameSpace KSaneIface
