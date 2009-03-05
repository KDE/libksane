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
#include <QList>
#include <QVector>
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

    QList<SelectionItem *>    selectionList;
    SelectionItem::Intersects change;

    QPointF lastSPoint;
    int m_left_last_x;
    int m_left_last_y;

    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomSelAction;
    QAction *zoom2FitAction;
    QAction *clrSelAction;
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
    d->selection->setSaved(false);

    d->scene->addItem(d->pixmapItem);
    d->scene->addItem(d->selection);
    d->scene->setBackgroundBrush(Qt::gray);

    d->change = SelectionItem::None;
    d->selectionList.clear();

    // create context menu
    d->zoomInAction = new QAction(KIcon("zoom-in"), i18n("Zoom In"), this);
    connect(d->zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
    
    d->zoomOutAction = new QAction(KIcon("zoom-out"), i18n("Zoom Out"), this);
    connect(d->zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));
    
    d->zoomSelAction = new QAction(KIcon("zoom-fit-best"), i18n("Zoom to Selection"), this);
    connect(d->zoomSelAction, SIGNAL(triggered()), this, SLOT(zoomSel()));
    
    d->zoom2FitAction = new QAction(KIcon("document-preview"), i18n("Zoom to Fit"), this);
    connect(d->zoom2FitAction, SIGNAL(triggered()), this, SLOT(zoom2Fit()));
    
    d->clrSelAction = new QAction(KIcon("edit-clear"), i18n("Clear Selections"), this);
    connect(d->clrSelAction, SIGNAL(triggered()), this, SLOT(clearSelections()));
    
    addAction(d->zoomInAction);
    addAction(d->zoomOutAction);
    addAction(d->zoomSelAction);
    addAction(d->zoom2FitAction);
    addAction(d->clrSelAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

// ------------------------------------------------------------------------
KSaneViewer::~KSaneViewer()
{
    // first remove any old saved selections
    clearSavedSelections();

    delete d;
}

// ------------------------------------------------------------------------
void KSaneViewer::setQImage(QImage *img)
{
    if (img == 0) return;

    // first remove any old saved selections
    clearSavedSelections();

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
    d->selection->saveZoom(transform().m11());
    for (int i=0; i<d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomOut()
{
    scale(1.0 / 1.5, 1.0 / 1.5);
    d->selection->saveZoom(transform().m11());
    for (int i=0; i<d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomSel()
{
    if (d->selection->isVisible()) {
        fitInView(d->selection->boundingRect() , Qt::KeepAspectRatio);
        d->selection->saveZoom(transform().m11());
        for (int i=0; i<d->selectionList.size(); ++i) {
            d->selectionList[i]->saveZoom(transform().m11());
        }
    }
    else {
        zoom2Fit();
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoom2Fit()
{
    fitInView(d->pixmapItem->boundingRect(), Qt::KeepAspectRatio);
    d->selection->saveZoom(transform().m11());
    for (int i=0; i<d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLX(float ratio)
{
    QRectF rect = d->selection->rect();
    rect.setLeft(ratio * d->pixmapItem->pixmap().width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLY(float ratio)
{
    QRectF rect = d->selection->rect();
    rect.setTop(ratio * d->pixmapItem->pixmap().height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRX(float ratio)
{
    QRectF rect = d->selection->rect();
    rect.setRight(ratio * d->pixmapItem->pixmap().width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRY(float ratio)
{
    QRectF rect = d->selection->rect();
    rect.setBottom(ratio * d->pixmapItem->pixmap().height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setSelection(float tl_x, float tl_y, float br_x, float br_y)
{
    QRectF rect;
    rect.setCoords(tl_x * d->pixmapItem->pixmap().width(),
                    tl_y * d->pixmapItem->pixmap().height(),
                    br_x * d->pixmapItem->pixmap().width(),
                    br_y * d->pixmapItem->pixmap().height());

    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::updateSelVisibility()
{
    if ((d->selection->rect().width() >0.001) &&
        (d->selection->rect().height() > 0.001) &&
        (d->pixmapItem->pixmap().width() - d->selection->rect().width() > 0.001) &&
        (d->pixmapItem->pixmap().height() - d->selection->rect().height() > 0.001))
    {
        d->selection->setVisible(true);
    }
    else {
        d->selection->setVisible(false);
    }
}

// ------------------------------------------------------------------------
int KSaneViewer::selListSize() {
    return d->selectionList.size();
}
// ------------------------------------------------------------------------
bool KSaneViewer::selectionAt(int index, float &tl_x, float &tl_y, float &br_x, float &br_y)
{
    if ((index < 0) || (index >= d->selectionList.size())) {
        tl_x = br_x;
        return false;
    }
    tl_x = d->selectionList[index]->rect().left()   / d->pixmapItem->pixmap().width();
    tl_y = d->selectionList[index]->rect().top()    / d->pixmapItem->pixmap().height();
    br_x = d->selectionList[index]->rect().right()  / d->pixmapItem->pixmap().width();
    br_y = d->selectionList[index]->rect().bottom() / d->pixmapItem->pixmap().height();
    return true;
}

// ------------------------------------------------------------------------
bool KSaneViewer::activeSelection(float &tl_x, float &tl_y, float &br_x, float &br_y)
{
    if (!d->selection->isVisible()) {
        tl_x = br_x; // just incase
        return false;
    }
    tl_x = d->selection->rect().left()   / d->pixmapItem->pixmap().width();
    tl_y = d->selection->rect().top()    / d->pixmapItem->pixmap().height();
    br_x = d->selection->rect().right()  / d->pixmapItem->pixmap().width();
    br_y = d->selection->rect().bottom() / d->pixmapItem->pixmap().height();
    if ((tl_x == br_x) || (tl_y == br_y)) return false; // FIXME this should not be needed
    return true;
}

// ------------------------------------------------------------------------
void KSaneViewer::clearActiveSelection()
{
    d->selection->setRect(QRectF(0,0,0,0));
    d->selection->intersects(QPointF(100,100)); // don't show the add sign
    d->selection->setVisible(false);
}

// ------------------------------------------------------------------------
void KSaneViewer::clearSavedSelections()
{
    // first remove any old saved selections
    SelectionItem *tmp;
    while (!d->selectionList.isEmpty()) {
        tmp = d->selectionList.takeFirst();
        d->scene->removeItem(tmp);
        delete tmp;
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::clearSelections()
{
    clearActiveSelection();
    clearSavedSelections();
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
                d->selection->intersects(scenePoint); // just to disable add/remove
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
    bool removed = false;
    if (e->button() == Qt::LeftButton) {
        if ((d->selection->rect().width() < 0.001) ||
            (d->selection->rect().height() < 0.001))
        {
            emit newSelection(0.0,0.0, 0.0, 0.0);
            clearActiveSelection();
        }

        QPointF scenePoint = mapToScene(e->pos());
        for (int i=0; i<d->selectionList.size(); i++) {
            if (d->selectionList[i]->intersects(scenePoint) == SelectionItem::AddRemove) {
                d->scene->removeItem(d->selectionList[i]);
                SelectionItem *tmp = d->selectionList[i];
                d->selectionList.removeAt(i);
                d->selection->setVisible(true);
                d->selection->setRect(tmp->rect());
                d->selection->intersects(scenePoint); // just to enable add/remove
                delete tmp;
                removed = true;
                break;
            }
        }
        if (!removed && (d->selection->intersects(scenePoint) == SelectionItem::AddRemove)) {
            // add the current selection
            SelectionItem *tmp = new SelectionItem(d->selection->rect());
            d->selectionList.push_back(tmp);
            d->selectionList.back()->setSaved(true);
            d->selectionList.back()->saveZoom(transform().m11());
            d->scene->addItem(d->selectionList.back());
            d->selectionList.back()->setZValue(9);
            d->selectionList.back()->intersects(scenePoint);

            // clear the old one
            emit newSelection(0.0,0.0, 0.0, 0.0);
            clearActiveSelection();
        }
    }
    
    if ((e->modifiers() != Qt::ControlModifier) &&
        (d->selection->isVisible()) &&
        (d->pixmapItem->pixmap().width() > 0.001) &&
        (d->pixmapItem->pixmap().height() > 0.001))
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
                case SelectionItem::AddRemove:
                    // do nothing
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
            case SelectionItem::AddRemove:
                setCursor(Qt::ArrowCursor);
                break;
        }
    }
    else {
        setCursor(Qt::CrossCursor);
    }

    // now check the selection list
    for (int i=0; i<d->selectionList.size(); i++) {
        if (d->selectionList[i]->intersects(scenePoint) == SelectionItem::AddRemove) {
            setCursor(Qt::ArrowCursor);
        }
    }

    d->lastSPoint = scenePoint;
    QGraphicsView::mouseMoveEvent(e);
}

// ------------------------------------------------------------------------
void KSaneViewer::findSelections()
{
    QVector<int> rowSums(d->img->height());
    QVector<int> colSums(d->img->width());
    rowSums.fill(0);
    colSums.fill(0);
    int pix;
    int average;
    int diff = 0;
    int y_sel_start=2;
    int x_sel_start=2;
    for (int y=2; y<d->img->height()-2; y++) {
        for (int x=2; x<d->img->width()-2; x++) {
            pix = qGray(d->img->pixel(x, y));
            diff = 0;
            // how much does the pixel differ from the surrounding
            diff += qAbs(pix - qGray(d->img->pixel(x-1, y)));
            diff += qAbs(pix - qGray(d->img->pixel(x-2, y)));
            diff += qAbs(pix - qGray(d->img->pixel(x+1, y)));
            diff += qAbs(pix - qGray(d->img->pixel(x+2, y)));
            diff += qAbs(pix - qGray(d->img->pixel(x, y-1)));
            diff += qAbs(pix - qGray(d->img->pixel(x, y-2)));
            diff += qAbs(pix - qGray(d->img->pixel(x, y+1)));
            diff += qAbs(pix - qGray(d->img->pixel(x, y+2)));
            if (diff > 30 ) { // a limit to tweek
                rowSums[y] += diff;
                colSums[x] += diff;
            }
        }
        average = rowSums[y-2]+rowSums[y-1]+rowSums[y]+rowSums[y+1]+rowSums[y+2];
        if (average < d->img->width()) { // this limit seems to be OK here.
            if ((y-y_sel_start) > 20) {
                // the selection ends here Y
                x_sel_start = 2;
                for (int x=2; x<d->img->width()-2; x++) {
                    average = colSums[x-1]+colSums[x-2]+colSums[x]+colSums[x+1]+colSums[x+2];
                    if (average < (y-y_sel_start)*2){ // *2 seems OK here.
                        // the selection ends here X
                        if ((x-x_sel_start) > 20) {
                            SelectionItem *tmp = new SelectionItem(QRect(x_sel_start, y_sel_start,
                                                                          x-x_sel_start, y-y_sel_start));
                            d->selectionList.push_back(tmp);
                            d->selectionList.back()->setSaved(true);
                            d->selectionList.back()->saveZoom(transform().m11());
                            d->scene->addItem(d->selectionList.back());
                            d->selectionList.back()->setZValue(9);
                        }
                        x_sel_start = x;
                    }
                }
            }
            if ((y-y_sel_start) > 2) {
                colSums.fill(0);
            }
            y_sel_start = y;
        }
    }
}

}  // NameSpace KSaneIface
