/*
 * SPDX-FileCopyrightText: 2008 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ksaneviewer.h"

#include "selectionitem.h"
#include "hiderectitem.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QAction>
#include <QList>
#include <QVector>
#include <QIcon>

#include <KLocalizedString>

#include <math.h>

namespace KSaneIface
{

struct KSaneViewer::Private {
    QGraphicsScene      *scene;
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

    HideRectItem *hideLeft;
    HideRectItem *hideRight;
    HideRectItem *hideTop;
    HideRectItem *hideBottom;
    HideRectItem *hideArea;

    bool multiSelectionEnabled = true;

    int wheelDelta = 0;

    int currentImageWidth;
    int currentImageHeight;

    QGraphicsPolygonItem * border;
};

KSaneViewer::KSaneViewer(QImage *img, QWidget *parent) : QGraphicsView(parent), d(new Private)
{
    d->img = img;

    setMouseTracking(true);

    // Init the scene
    d->scene = new QGraphicsScene(this);
    const auto dpr = img->devicePixelRatio();

    d->currentImageWidth = img->width();
    d->currentImageHeight = img->height();

    d->scene->setSceneRect(0, 0, d->currentImageWidth / dpr, d->currentImageHeight / dpr);
    setScene(d->scene);

    d->selection = new SelectionItem(QRectF());
    d->selection->setZValue(10);
    d->selection->setSaved(false);
    d->selection->setMaxRight(d->currentImageWidth);
    d->selection->setMaxBottom(d->currentImageHeight);
    d->selection->setRect(d->scene->sceneRect());
    d->selection->setVisible(false);

    d->hideTop = new HideRectItem;
    d->hideBottom = new HideRectItem;
    d->hideRight = new HideRectItem;
    d->hideLeft = new HideRectItem;
    d->hideArea = new HideRectItem;
    d->hideArea->setOpacity(0.6);

    d->scene->addItem(d->selection);
    d->scene->addItem(d->hideLeft);
    d->scene->addItem(d->hideRight);
    d->scene->addItem(d->hideTop);
    d->scene->addItem(d->hideBottom);
    d->scene->addItem(d->hideArea);

    QPolygonF polygon(QRectF(QPointF(0,0),QSizeF(d->currentImageWidth, d->currentImageHeight)));
    QPen pen(Qt::gray, 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    d->border = d->scene->addPolygon(polygon, pen);

    d->change = SelectionItem::None;
    d->selectionList.clear();

    // create context menu
    d->zoomInAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-in")), i18n("Zoom In"), this);
    connect(d->zoomInAction, &QAction::triggered, this, &KSaneViewer::zoomIn);

    d->zoomOutAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-out")), i18n("Zoom Out"), this);
    connect(d->zoomOutAction, &QAction::triggered, this, &KSaneViewer::zoomOut);

    d->zoomSelAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")), i18n("Zoom to Selection"), this);
    connect(d->zoomSelAction, &QAction::triggered, this, &KSaneViewer::zoomSel);

    d->zoom2FitAction = new QAction(QIcon::fromTheme(QLatin1String("document-preview")), i18n("Zoom to Fit"), this);
    connect(d->zoom2FitAction, &QAction::triggered, this, &KSaneViewer::zoom2Fit);

    d->clrSelAction = new QAction(QIcon::fromTheme(QLatin1String("edit-clear")), i18n("Clear Selections"), this);
    connect(d->clrSelAction, &QAction::triggered, this, &KSaneViewer::clearSelections);

    addAction(d->zoomInAction);
    addAction(d->zoomOutAction);
    addAction(d->zoomSelAction);
    addAction(d->zoom2FitAction);
    addAction(d->clrSelAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    setFrameShape(QFrame::NoFrame);
}

// ------------------------------------------------------------------------
void KSaneViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QWidget::palette().color(QPalette::Window));
    QRectF r = rect & sceneRect();
    const qreal dpr = d->img->devicePixelRatio();
    QRectF srcRect = QRectF(r.topLeft() * dpr, r.size() * dpr);
    painter->drawImage(r, *d->img, srcRect);
    d->border->setPolygon(QPolygonF(QRectF(QPointF(0,0),QSizeF(d->currentImageWidth, d->currentImageHeight))));
}

// ------------------------------------------------------------------------
KSaneViewer::~KSaneViewer()
{
    // first remove any old saved selections
    clearSavedSelections();

    delete d;
}

// ------------------------------------------------------------------------
int KSaneViewer::currentImageHeight() const
{
    return d->currentImageHeight;
}

// ------------------------------------------------------------------------
int KSaneViewer::currentImageWidth() const
{
    return d->currentImageWidth;
}

// ------------------------------------------------------------------------
void KSaneViewer::setQImage(QImage *img)
{
    if (img == nullptr) {
        return;
    }

    // remove selections
    clearSelections();

    // clear zoom
    resetTransform();

    const auto dpr = img->devicePixelRatio();

    d->currentImageWidth = img->width();
    d->currentImageHeight = img->height();

    d->scene->setSceneRect(0, 0, d->currentImageWidth / dpr, d->currentImageHeight / dpr);
    d->selection->setMaxRight(d->currentImageWidth);
    d->selection->setMaxBottom(d->currentImageHeight);

    d->selection->setDevicePixelRatio(dpr);
    d->hideTop->setDevicePixelRatio(dpr);
    d->hideBottom->setDevicePixelRatio(dpr);
    d->hideRight->setDevicePixelRatio(dpr);
    d->hideLeft->setDevicePixelRatio(dpr);
    d->hideArea->setDevicePixelRatio(dpr);

    d->img = img;
}

// ------------------------------------------------------------------------
void KSaneViewer::updateImage()
{
    setCacheMode(QGraphicsView::CacheNone);
    repaint();
    setCacheMode(QGraphicsView::CacheBackground);
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomIn()
{
    scale(1.3, 1.3);
    d->selection->saveZoom(transform().m11());
    for (int i = 0; i < d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomOut()
{
    scale(1.0 / 1.3, 1.0 / 1.3);
    d->selection->saveZoom(transform().m11());
    for (int i = 0; i < d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoomSel()
{
    if (d->selection->isVisible()) {
        fitInView(d->selection->boundingRect() , Qt::KeepAspectRatio);
        d->selection->saveZoom(transform().m11());
        for (int i = 0; i < d->selectionList.size(); ++i) {
            d->selectionList[i]->saveZoom(transform().m11());
        }
    } else {
        zoom2Fit();
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::zoom2Fit()
{
    fitInView(d->img->rect(), Qt::KeepAspectRatio);
    d->selection->saveZoom(transform().m11());
    for (int i = 0; i < d->selectionList.size(); ++i) {
        d->selectionList[i]->saveZoom(transform().m11());
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLX(float ratio)
{
    if (!d->selection->isVisible()) {
        return;    // only correct the selection if it is visible
    }
    QRectF rect = d->selection->rect();
    rect.setLeft(ratio * d->img->width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setTLY(float ratio)
{
    if (!d->selection->isVisible()) {
        return;    // only correct the selection if it is visible
    }
    QRectF rect = d->selection->rect();
    rect.setTop(ratio * d->img->height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRX(float ratio)
{
    if (!d->selection->isVisible()) {
        return;    // only correct the selection if it is visible
    }
    QRectF rect = d->selection->rect();
    rect.setRight(ratio * d->img->width());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setBRY(float ratio)
{
    if (!d->selection->isVisible()) {
        return;    // only correct the selection if it is visible
    }
    QRectF rect = d->selection->rect();
    rect.setBottom(ratio * d->img->height());
    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setSelection(float tl_x, float tl_y, float br_x, float br_y)
{
    QRectF rect;
    rect.setCoords(tl_x * d->img->width(),
                   tl_y * d->img->height(),
                   br_x * d->img->width(),
                   br_y * d->img->height());

    d->selection->setRect(rect);
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setHighlightArea(float tl_x, float tl_y, float br_x, float br_y)
{
    QRectF rect;

    // Left  reason for rect: setCoords(x1,y1,x2,y2) != setRect(x1,x2, width, height)
    rect.setCoords(0, 0, tl_x * d->img->width(), d->img->height());
    d->hideLeft->setRect(rect);

    // Right
    rect.setCoords(br_x * d->img->width(),
                   0,
                   d->img->width(),
                   d->img->height());
    d->hideRight->setRect(rect);

    // Top
    rect.setCoords(tl_x * d->img->width(),
                   0,
                   br_x * d->img->width(),
                   tl_y * d->img->height());
    d->hideTop->setRect(rect);

    // Bottom
    rect.setCoords(tl_x * d->img->width(),
                   br_y * d->img->height(),
                   br_x * d->img->width(),
                   d->img->height());
    d->hideBottom->setRect(rect);

    // hide area
    rect.setCoords(tl_x * d->img->width(), tl_y * d->img->height(),
                   br_x * d->img->width(), br_y * d->img->height());

    d->hideArea->setRect(rect);

    d->hideLeft->show();
    d->hideRight->show();
    d->hideTop->show();
    d->hideBottom->show();
    // the hide area is hidden until setHighlightShown is called.
    d->hideArea->hide();
}

// ------------------------------------------------------------------------
void KSaneViewer::setHighlightShown(int percentage, QColor hideColor)
{
    if (percentage >= 100) {
        d->hideArea->hide();
        return;
    }

    d->hideArea->setBrush(hideColor);

    qreal diff = d->hideBottom->rect().top() - d->hideTop->rect().bottom();
    diff -= (diff * percentage) / 100;

    QRectF rect = d->hideArea->rect();
    rect.setTop(d->hideBottom->rect().top() - diff);

    d->hideArea->setRect(rect);

    d->hideArea->show();
}

// ------------------------------------------------------------------------
void KSaneViewer::updateHighlight()
{
    if (d->selection->isVisible()) {
        QRectF rect;
        // Left
        rect.setCoords(0, 0, d->selection->rect().left(), d->img->height());
        d->hideLeft->setRect(rect);

        // Right
        rect.setCoords(d->selection->rect().right(),
                       0,
                       d->img->width(),
                       d->img->height());
        d->hideRight->setRect(rect);

        // Top
        rect.setCoords(d->selection->rect().left(),
                       0,
                       d->selection->rect().right(),
                       d->selection->rect().top());
        d->hideTop->setRect(rect);

        // Bottom
        rect.setCoords(d->selection->rect().left(),
                       d->selection->rect().bottom(),
                       d->selection->rect().right(),
                       d->img->height());
        d->hideBottom->setRect(rect);

        d->hideLeft->show();
        d->hideRight->show();
        d->hideTop->show();
        d->hideBottom->show();
        d->hideArea->hide();
    } else {
        d->hideLeft->hide();
        d->hideRight->hide();
        d->hideTop->hide();
        d->hideBottom->hide();
        d->hideArea->hide();
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::clearHighlight()
{
    d->hideLeft->hide();
    d->hideRight->hide();
    d->hideTop->hide();
    d->hideBottom->hide();
    d->hideArea->hide();
}

// ------------------------------------------------------------------------
void KSaneViewer::updateSelVisibility()
{
    if ((d->selection->rect().width() > 0.001) &&
            (d->selection->rect().height() > 0.001) &&
            ((d->img->width() - d->selection->rect().width() > 0.1) ||
             (d->img->height() - d->selection->rect().height() > 0.1))) {
        d->selection->setVisible(true);
    } else {
        d->selection->setVisible(false);
    }
    updateHighlight();
}

// ---- Return the saved selection list size + 1 if the selection is visible -
int KSaneViewer::selListSize()
{
    if (d->selection->isVisible()) {
        return (d->selectionList.size() + 1);
    } else {
        return d->selectionList.size();
    }
}

// ---- First return the "saved" selection sthen the active selection -----------
bool KSaneViewer::selectionAt(int index, float &tl_x, float &tl_y, float &br_x, float &br_y)
{
    if ((index < 0) || (index > d->selectionList.size())) {
        activeSelection(tl_x, tl_y, br_x, br_y);
        return false;
    }
    if (index == d->selectionList.size()) {
        return activeSelection(tl_x, tl_y, br_x, br_y);
    }

    tl_x = d->selectionList[index]->rect().left()   / d->img->width();
    tl_y = d->selectionList[index]->rect().top()    / d->img->height();
    br_x = d->selectionList[index]->rect().right()  / d->img->width();
    br_y = d->selectionList[index]->rect().bottom() / d->img->height();
    return true;
}

// ------------------------------------------------------------------------
bool KSaneViewer::activeSelection(float &tl_x, float &tl_y, float &br_x, float &br_y)
{
    if (!d->selection->isVisible()) {
        tl_x = 0.0;
        tl_y = 0.0;
        br_x = 1.0;
        br_y = 1.0;
        return true;
    }

    tl_x = d->selection->rect().left()   / d->img->width();
    tl_y = d->selection->rect().top()    / d->img->height();
    br_x = d->selection->rect().right()  / d->img->width();
    br_y = d->selection->rect().bottom() / d->img->height();

    if ((tl_x == br_x) || (tl_y == br_y)) {
        tl_x = 0.0;
        tl_y = 0.0;
        br_x = 1.0;
        br_y = 1.0;
        return false; // just precaution
    }
    return true;
}

// ------------------------------------------------------------------------
void KSaneViewer::clearActiveSelection()
{
    d->selection->setRect(QRectF(0, 0, 0, 0));
    d->selection->intersects(QPointF(100, 100)); // don't show the add sign
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
    updateSelVisibility();
}

// ------------------------------------------------------------------------
void KSaneViewer::setMultiselectionEnabled(bool enabled)
{
    d->multiSelectionEnabled = enabled;
    clearSelections();
    d->selection->setAddButtonEnabled(enabled);
}


// ------------------------------------------------------------------------
void KSaneViewer::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier) {
        d->wheelDelta +=  e->angleDelta().y();

        while (d->wheelDelta >= QWheelEvent::DefaultDeltasPerStep) {
            zoomIn();
            d->wheelDelta -= QWheelEvent::DefaultDeltasPerStep;
        }

        while (d->wheelDelta <= -QWheelEvent::DefaultDeltasPerStep) {
            zoomOut();
            d->wheelDelta += QWheelEvent::DefaultDeltasPerStep;
        }
    } else {
        QGraphicsView::wheelEvent(e);
    }
}

// ------------------------------------------------------------------------
void KSaneViewer::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        d->m_left_last_x = e->pos().x();
        d->m_left_last_y = e->pos().y();
        QPointF scenePoint = scenePos(e) * d->selection->devicePixelRatio();
        d->lastSPoint = scenePoint;
        if (e->modifiers() != Qt::ControlModifier) {
            if (!d->selection->isVisible()) {
                d->selection->setVisible(true);
                d->selection->setRect(QRectF(scenePoint, QSizeF(0, 0)));
                d->selection->intersects(scenePoint); // just to disable add/remove
                d->change = SelectionItem::BottomRight;
            } else if (d->selection->intersects(scenePoint) == SelectionItem::None) {
                d->selection->setRect(QRectF(scenePoint, QSizeF(0, 0)));
                d->change = SelectionItem::BottomRight;
            }
            updateHighlight();
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
                (d->selection->rect().height() < 0.001)) {
            Q_EMIT newSelection(0.0, 0.0, 1.0, 1.0);
            clearActiveSelection();
        }

        QPointF scenePoint = scenePos(e) * d->selection->devicePixelRatio();
        for (int i = 0; i < d->selectionList.size(); i++) {
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
            tmp->setDevicePixelRatio(d->img->devicePixelRatio());
            d->selectionList.push_back(tmp);
            d->selectionList.back()->setSaved(true);
            d->selectionList.back()->saveZoom(transform().m11());
            d->scene->addItem(d->selectionList.back());
            d->selectionList.back()->setZValue(9);
            d->selectionList.back()->intersects(scenePoint);

            // clear the old one
            Q_EMIT newSelection(0.0, 0.0, 1.0, 1.0);
            clearActiveSelection();
        }
    }

    if ((e->modifiers() != Qt::ControlModifier) &&
            (d->selection->isVisible()) &&
            (d->img->width() > 0.001) &&
            (d->img->height() > 0.001)) {
        float tlx = d->selection->rect().left()   / d->img->width();
        float tly = d->selection->rect().top()    / d->img->height();
        float brx = d->selection->rect().right()  / d->img->width();
        float bry = d->selection->rect().bottom() / d->img->height();

        Q_EMIT newSelection(tlx, tly, brx, bry);
    }
    updateHighlight();
    QGraphicsView::mouseReleaseEvent(e);
}

// ------------------------------------------------------------------------
void KSaneViewer::mouseMoveEvent(QMouseEvent *e)
{
    QPointF scenePoint = scenePos(e) * d->selection->devicePixelRatio();

    if (e->buttons()&Qt::LeftButton) {
        if (e->modifiers() == Qt::ControlModifier) {
            int dx = e->pos().x() - d->m_left_last_x;
            int dy = e->pos().y() - d->m_left_last_y;
            verticalScrollBar()->setValue(verticalScrollBar()->value() - dy);
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - dx);
            d->m_left_last_x = e->pos().x();
            d->m_left_last_y = e->pos().y();
        } else {
            ensureVisible(QRectF(scenePoint, QSizeF(0, 0)), 1, 1);
            QRectF rect = d->selection->rect();
            switch (d->change) {
            case SelectionItem::None:
                // should not be here :)
                break;
            case SelectionItem::Top:
                if (scenePoint.y() < rect.bottom()) {
                    rect.setTop(scenePoint.y());
                } else {
                    d->change = SelectionItem::Bottom;
                    rect.setBottom(scenePoint.y());
                }
                break;
            case SelectionItem::TopRight:
                if (scenePoint.x() > rect.left()) {
                    rect.setRight(scenePoint.x());
                } else {
                    rect.setLeft(scenePoint.x());
                    d->change = SelectionItem::TopLeft;
                }
                if (scenePoint.y() < rect.bottom()) {
                    rect.setTop(scenePoint.y());
                } else {
                    rect.setBottom(scenePoint.y());
                    d->change = SelectionItem::BottomLeft;
                } // FIXME arrow
                break;
            case SelectionItem::Right:
                if (scenePoint.x() > rect.left()) {
                    rect.setRight(scenePoint.x());
                } else {
                    rect.setLeft(scenePoint.x());
                    d->change = SelectionItem::Left;
                }
                break;
            case SelectionItem::BottomRight:
                if (scenePoint.x() > rect.left()) {
                    rect.setRight(scenePoint.x());
                } else {
                    rect.setLeft(scenePoint.x());
                    d->change = SelectionItem::BottomLeft;
                }
                if (scenePoint.y() > rect.top()) {
                    rect.setBottom(scenePoint.y());
                } else {
                    rect.setTop(scenePoint.y());
                    d->change = SelectionItem::TopRight;
                } // FIXME arrow
                break;
            case SelectionItem::Bottom:
                if (scenePoint.y() > rect.top()) {
                    rect.setBottom(scenePoint.y());
                } else {
                    d->change = SelectionItem::Top;
                    rect.setTop(scenePoint.y());
                }
                break;
            case SelectionItem::BottomLeft:
                if (scenePoint.x() < rect.right()) {
                    rect.setLeft(scenePoint.x());
                } else {
                    rect.setRight(scenePoint.x());
                    d->change = SelectionItem::BottomRight;
                }
                if (scenePoint.y() > rect.top()) {
                    rect.setBottom(scenePoint.y());
                } else {
                    rect.setTop(scenePoint.y());
                    d->change = SelectionItem::TopLeft;
                } // FIXME arrow
                break;
            case SelectionItem::Left:
                if (scenePoint.x() < rect.right()) {
                    rect.setLeft(scenePoint.x());
                } else {
                    rect.setRight(scenePoint.x());
                    d->change = SelectionItem::Right;
                }
                break;
            case SelectionItem::TopLeft:
                if (scenePoint.x() < rect.right()) {
                    rect.setLeft(scenePoint.x());
                } else {
                    rect.setRight(scenePoint.x());
                    d->change = SelectionItem::TopRight;
                }
                if (scenePoint.y() < rect.bottom()) {
                    rect.setTop(scenePoint.y());
                } else {
                    rect.setBottom(scenePoint.y());
                    d->change = SelectionItem::BottomLeft;
                }// FIXME arrow
                break;
            case SelectionItem::Move:
                rect.translate(d->selection->fixTranslation(scenePoint - d->lastSPoint));
                break;
            case SelectionItem::AddRemove:
                // do nothing
                break;
            }
            d->selection->setRect(rect);
        }
    } else if (d->selection->isVisible()) {
        d->change = d->selection->intersects(scenePoint);

        switch (d->change) {
        case SelectionItem::None:
            viewport()->setCursor(Qt::CrossCursor);
            break;
        case SelectionItem::Top:
            viewport()->setCursor(Qt::SizeVerCursor);
            break;
        case SelectionItem::TopRight:
            viewport()->setCursor(Qt::SizeBDiagCursor);
            break;
        case SelectionItem::Right:
            viewport()->setCursor(Qt::SizeHorCursor);
            break;
        case SelectionItem::BottomRight:
            viewport()->setCursor(Qt::SizeFDiagCursor);
            break;
        case SelectionItem::Bottom:
            viewport()->setCursor(Qt::SizeVerCursor);
            break;
        case SelectionItem::BottomLeft:
            viewport()->setCursor(Qt::SizeBDiagCursor);
            break;
        case SelectionItem::Left:
            viewport()->setCursor(Qt::SizeHorCursor);
            break;
        case SelectionItem::TopLeft:
            viewport()->setCursor(Qt::SizeFDiagCursor);
            break;
        case SelectionItem::Move:
            viewport()->setCursor(Qt::SizeAllCursor);
            break;
        case SelectionItem::AddRemove:
            viewport()->setCursor(Qt::ArrowCursor);
            break;
        }
    } else {
        viewport()->setCursor(Qt::CrossCursor);
    }

    // now check the selection list
    for (int i = 0; i < d->selectionList.size(); i++) {
        if (d->selectionList[i]->intersects(scenePoint) == SelectionItem::AddRemove) {
            viewport()->setCursor(Qt::ArrowCursor);
        }
    }

    d->lastSPoint = scenePoint;
    updateHighlight();
    QGraphicsView::mouseMoveEvent(e);
}

// The change trigger before adding to the sum
static const int DIFF_TRIGGER = 8;

// The selection start/stop level trigger
static const int SUM_TRIGGER = 4;

// The selection start/stop level trigger for the floating  average
static const int AVERAGE_TRIGGER = 7;

// The selection start/stop margin
static const int SEL_MARGIN = 3;

// Maximum number of allowed selections (this could be a settable variable)
static const int MAX_NUM_SELECTIONS = 8;

// floating average 'div' must be one less than 'count'
static const int AVERAGE_COUNT = 50;
static const int AVERAGE_MULT = 49;

// Minimum selection area compared to the whole image
static const float MIN_AREA_SIZE = 0.01;
// ------------------------------------------------------------------------
void KSaneViewer::findSelections(float area)
{
    // Reduce the size of the image to decrease noise and calculation time
    float multiplier = sqrt(area / (d->img->height() * d->img->width()));

    int width  = (int)(d->img->width() * multiplier);
    int height = (int)(d->img->height() * multiplier);

    QImage img = d->img->scaled(width, height, Qt::KeepAspectRatio);
    height = img.height(); // the size was probably not exact
    width  = img.width();

    QVector<qint64> colSums(width + SEL_MARGIN + 1);
    qint64 rowSum;
    colSums.fill(0);
    int pix;
    int diff;
    int hSelStart = -1;
    int hSelEnd = -1;
    int hSelMargin = 0;
    int wSelStart = -1;
    int wSelEnd = -1;
    int wSelMargin = 0;

    for (int h = 1; h < height; h++) {
        rowSum = 0;
        if (h < height - 1) {
            // Special case for the left most pixel
            pix = qGray(img.pixel(0, h));
            diff  = qAbs(pix - qGray(img.pixel(1, h)));
            diff += qAbs(pix - qGray(img.pixel(0, h - 1)));
            diff += qAbs(pix - qGray(img.pixel(0, h + 1)));
            if (diff > DIFF_TRIGGER) {
                colSums[0] += diff;
                rowSum += diff;
            }

            // Special case for the right most pixel
            pix = qGray(img.pixel(width - 1, h));
            diff  = qAbs(pix - qGray(img.pixel(width - 2, h)));
            diff += qAbs(pix - qGray(img.pixel(width - 1, h - 1)));
            diff += qAbs(pix - qGray(img.pixel(width - 1, h + 1)));
            if (diff > DIFF_TRIGGER) {
                colSums[width - 1] += diff;
                rowSum += diff;
            }

            for (int w = 1; w < (width - 1); w++) {
                pix = qGray(img.pixel(w, h));
                diff = 0;
                // how much does the pixel differ from the surrounding
                diff += qAbs(pix - qGray(img.pixel(w - 1, h)));
                diff += qAbs(pix - qGray(img.pixel(w + 1, h)));
                diff += qAbs(pix - qGray(img.pixel(w, h - 1)));
                diff += qAbs(pix - qGray(img.pixel(w, h + 1)));
                if (diff > DIFF_TRIGGER) {
                    colSums[w] += diff;
                    rowSum += diff;
                }
            }
        }

        if ((rowSum / width) > SUM_TRIGGER) {
            if (hSelStart < 0) {
                if (hSelMargin < SEL_MARGIN) {
                    hSelMargin++;
                }
                if (hSelMargin == SEL_MARGIN) {
                    hSelStart = h - SEL_MARGIN + 1;
                }
            }
        } else {
            if (hSelStart >= 0) {
                if (hSelMargin > 0) {
                    hSelMargin--;
                }
            }
            if ((hSelStart > -1) && ((hSelMargin == 0) || (h == height - 1))) {
                if (h == height - 1) {
                    hSelEnd = h - hSelMargin;
                } else {
                    hSelEnd = h - SEL_MARGIN;
                }
                // We have the end of the vertical selection
                // now figure out the horizontal part of the selection
                for (int w = 0; w <= width; w++) { // colSums[width] will be 0
                    if ((colSums[w] / (h - hSelStart)) > SUM_TRIGGER) {
                        if (wSelStart < 0) {
                            if (wSelMargin < SEL_MARGIN) {
                                wSelMargin++;
                            }
                            if (wSelMargin == SEL_MARGIN) {
                                wSelStart = w - SEL_MARGIN + 1;
                            }
                        }
                    } else {
                        if (wSelStart >= 0) {
                            if (wSelMargin > 0) {
                                wSelMargin--;
                            }
                        }
                        if ((wSelStart >= 0) && ((wSelMargin == 0) || (w == width))) {
                            if (w == width) {
                                wSelEnd = width;
                            } else {
                                wSelEnd = w - SEL_MARGIN + 1;
                            }

                            // we have the end of a horizontal selection
                            if ((wSelEnd - wSelStart) < width) {
                                // skip selections that span the whole width
                                // calculate the coordinates in the original size
                                int x1 = wSelStart / multiplier;
                                int y1 = hSelStart / multiplier;
                                int x2 = wSelEnd / multiplier;
                                int y2 = hSelEnd / multiplier;
                                float selArea = (float)(wSelEnd - wSelStart) * (float)(hSelEnd - hSelStart);
                                if (selArea > (area * MIN_AREA_SIZE)) {
                                    SelectionItem *tmp = new SelectionItem(QRect(QPoint(x1, y1), QPoint(x2, y2)));
                                    tmp->setDevicePixelRatio(d->img->devicePixelRatio());
                                    d->selectionList.push_back(tmp);
                                    d->selectionList.back()->setSaved(true);
                                    d->selectionList.back()->saveZoom(transform().m11());
                                    d->scene->addItem(d->selectionList.back());
                                    d->selectionList.back()->setZValue(9);
                                }
                            }
                            wSelStart = -1;
                            wSelEnd = -1;
                            (void)hSelEnd;    // clang static analyzer report hSelEnd is never used.
                            wSelMargin = 0;
                        }
                    }
                }
                hSelStart = -1;
                hSelEnd = -1;
                hSelMargin = 0;
                colSums.fill(0);
            }
        }
    }

    if (d->selectionList.size() > MAX_NUM_SELECTIONS) {
        // smaller area or should we give up??
        clearSavedSelections();
        //findSelections(area/2);
        // instead of trying to find probably broken selections just give up
        // and do not force broken selections on the user.
    } else {
        // 1/multiplier is the error margin caused by the resolution reduction
        refineSelections(qRound(1 / multiplier));
        // check that the selections are big enough
        float minArea = d->img->height() * d->img->width() * MIN_AREA_SIZE;

        int i = 0;
        while (i < d->selectionList.size()) {
            if ((d->selectionList[i]->rect().width() * d->selectionList[i]->rect().height()) < minArea) {
                d->scene->removeItem(d->selectionList[i]);
                d->selectionList.removeAt(i);
            } else {
                i++;
            }
        }
    }
}

QSize KSaneViewer::sizeHint() const
{
    return QSize(250, 300);  // a sensible size for a scan preview
}

void KSaneViewer::refineSelections(int pixelMargin)
{
    // The end result
    int hSelStart;
    int hSelEnd;
    int wSelStart;
    int wSelEnd;

    for (int i = 0; i < d->selectionList.size(); i++) {
        QRectF selRect = d->selectionList.at(i)->rect();

        // original values
        hSelStart = (int)selRect.top();
        hSelEnd = (int)selRect.bottom();
        wSelStart = (int)selRect.left();
        wSelEnd = (int)selRect.right();

        // Top
        // Too long iteration should not be a problem since the loop should be interrupted by the limit
        hSelStart = refineRow(hSelStart - pixelMargin, hSelEnd, wSelStart, wSelEnd);

        // Bottom (from the bottom up wards)
        hSelEnd = refineRow(hSelEnd + pixelMargin, hSelStart, wSelStart, wSelEnd);

        // Left
        wSelStart = refineColumn(wSelStart - pixelMargin, wSelEnd, hSelStart, hSelEnd);

        // Right
        wSelEnd = refineColumn(wSelEnd + pixelMargin, wSelStart, hSelStart, hSelEnd);

        // Now update the selection
        d->selectionList.at(i)->setRect(QRectF(QPointF(wSelStart, hSelStart), QPointF(wSelEnd, hSelEnd)));
    }
}

int KSaneViewer::refineRow(int fromRow, int toRow, int colStart, int colEnd)
{
    int pix;
    int diff;
    float rowTrigger;
    int row;
    int addSub = (fromRow < toRow) ? 1 : -1;

    colStart -= 2; //add some margin
    colEnd += 2; //add some margin

    if (colStart < 1) {
        colStart = 1;
    }
    if (colEnd >= d->img->width() - 1) {
        colEnd = d->img->width() - 2;
    }

    if (fromRow < 1) {
        fromRow = 1;
    }
    if (fromRow >= d->img->height() - 1) {
        fromRow = d->img->height() - 2;
    }

    if (toRow < 1) {
        toRow = 1;
    }
    if (toRow >= d->img->height() - 1) {
        toRow = d->img->height() - 2;
    }

    row = fromRow;
    while (row != toRow) {
        rowTrigger = 0;
        for (int w = colStart; w < colEnd; w++) {
            diff = 0;
            pix = qGray(d->img->pixel(w, row));
            // how much does the pixel differ from the surrounding
            diff += qAbs(pix - qGray(d->img->pixel(w - 1, row)));
            diff += qAbs(pix - qGray(d->img->pixel(w + 1, row)));
            diff += qAbs(pix - qGray(d->img->pixel(w, row - 1)));
            diff += qAbs(pix - qGray(d->img->pixel(w, row + 1)));
            if (diff <= DIFF_TRIGGER) {
                diff = 0;
            }

            rowTrigger = ((rowTrigger * AVERAGE_MULT) + diff) / AVERAGE_COUNT;

            if (rowTrigger > AVERAGE_TRIGGER) {
                break;
            }
        }

        if (rowTrigger > AVERAGE_TRIGGER) {
            // row == 1 _probably_ means that the selection should start from 0
            // but that can not be detected if we start from 1 => include one extra column
            if (row == 1) {
                row = 0;
            }
            if (row == (d->img->width() - 2)) {
                row = d->img->width();
            }
            return row;
        }
        row += addSub;
    }
    return row;
}

int KSaneViewer::refineColumn(int fromCol, int toCol, int rowStart, int rowEnd)
{
    int pix;
    int diff;
    float colTrigger;
    int col;
    int count;
    int addSub = (fromCol < toCol) ? 1 : -1;

    rowStart -= 2; //add some margin
    rowEnd += 2; //add some margin

    if (rowStart < 1) {
        rowStart = 1;
    }
    if (rowEnd >= d->img->height() - 1) {
        rowEnd = d->img->height() - 2;
    }

    if (fromCol < 1) {
        fromCol = 1;
    }
    if (fromCol >= d->img->width() - 1) {
        fromCol = d->img->width() - 2;
    }

    if (toCol < 1) {
        toCol = 1;
    }
    if (toCol >= d->img->width() - 1) {
        toCol = d->img->width() - 2;
    }

    col = fromCol;
    while (col != toCol) {
        colTrigger = 0;
        count = 0;
        for (int row = rowStart; row < rowEnd; row++) {
            count++;
            diff = 0;
            pix = qGray(d->img->pixel(col, row));
            // how much does the pixel differ from the surrounding
            diff += qAbs(pix - qGray(d->img->pixel(col - 1, row)));
            diff += qAbs(pix - qGray(d->img->pixel(col + 1, row)));
            diff += qAbs(pix - qGray(d->img->pixel(col, row - 1)));
            diff += qAbs(pix - qGray(d->img->pixel(col, row + 1)));
            if (diff <= DIFF_TRIGGER) {
                diff = 0;
            }

            colTrigger = ((colTrigger * AVERAGE_MULT) + diff) / AVERAGE_COUNT;

            if (colTrigger > AVERAGE_TRIGGER) {
                break;
            }
        }

        if (colTrigger > AVERAGE_TRIGGER) {
            // col == 1 _probably_ means that the selection should start from 0
            // but that can not be detected if we start from 1 => include one extra column
            if (col == 1) {
                col = 0;
            }
            if (col == (d->img->width() - 2)) {
                col = d->img->width();
            }
            return col;
        }
        col += addSub;
    }
    return col;
}

QPointF KSaneViewer::scenePos(QMouseEvent *e) const
{
    // QGraphicsView::mapToScene() maps only QPoints, but in highdpi mode we want
    // to deal with non-rounded coordinates, that's why QPainterPath wrapper is used.
    // QMouseEvent::localPos() currently returns a rounded QPointF
    // (https://codereview.qt-project.org/259785), so we have to extract a fractional
    // part from QMouseEvent::screenPos().
    const QPointF screenPos = e->screenPos();
    QPointF delta = screenPos - screenPos.toPoint();
    return mapToScene(QPainterPath(e->pos() + delta)).currentPosition();
}

}  // NameSpace KSaneIface

#include "moc_ksaneviewer.cpp"
