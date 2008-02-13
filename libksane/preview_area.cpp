/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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

// Local includes.
#include "preview_area.h"
#include "preview_area.moc"

// Qt includes
#include <QScrollBar>

// KDE includes
#include <KDebug>

namespace KSaneIface
{

PreviewArea::PreviewArea(QWidget *parent)
           : QScrollArea(parent)
{
    image = new PreviewImage(this);
    setWidget(image);
    setBackgroundRole(QPalette::Dark);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(image, SIGNAL(newSelection(float,float,float,float)),
            this, SIGNAL(newSelection(float,float,float,float)));

    connect(image, SIGNAL(requestVisibility(int,int)),
            this, SLOT(requestVisibility(int,int)));

    createContextMenu();
}

PreviewArea::~PreviewArea()
{
}

void PreviewArea::createContextMenu()
{
    zoomInAction = new QAction(tr("Zoom In"), this);
    connect(zoomInAction, SIGNAL(triggered()), image, SLOT(zoomIn()));

    zoomOutAction = new QAction(tr("Zoom Out"), this);
    connect(zoomOutAction, SIGNAL(triggered()), image, SLOT(zoomOut()));

    zoomSelAction = new QAction(tr("Zoom to Selection"), this);
    connect(zoomSelAction, SIGNAL(triggered()), image, SLOT(zoomSel()));

    zoom2FitAction = new QAction(tr("Zoom to Fit"), this);
    connect(zoom2FitAction, SIGNAL(triggered()), image, SLOT(zoom2Fit()));

    addAction(zoomInAction);
    addAction(zoomOutAction);
    addAction(zoomSelAction);
    addAction(zoom2FitAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);
}

QSize PreviewArea::sizeHint() const
{
    return QSize(500, 600);
}

QImage *PreviewArea::getImage()
{
    if (image) {
        return image->getImage();
    }
    return 0;
}

void PreviewArea::clearSelection()
{
    if (image) {
        image->clearSelection();
    }
}

void PreviewArea::updateScaledImg()
{
    if (image) {
        image->updateScaledImg();
        image->update();
    }
}

void PreviewArea::zoomIn()
{
    if (image) {
        int h_max = horizontalScrollBar()->maximum();
        int v_max = verticalScrollBar()->maximum();

        image->zoomIn();

        int diff = (horizontalScrollBar()->maximum() - h_max) / 2;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + diff);

        diff = (verticalScrollBar()->maximum() - v_max) / 2;
        verticalScrollBar()->setValue(verticalScrollBar()->value() + diff);
    }
}

void PreviewArea::zoomOut()
{
    if (image) {
        int h_max = horizontalScrollBar()->maximum();
        int v_max = verticalScrollBar()->maximum();

        image->zoomOut();

        int diff = (h_max - horizontalScrollBar()->maximum()) / 2;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - diff);

        diff = (v_max - verticalScrollBar()->maximum()) / 2;
        verticalScrollBar()->setValue(verticalScrollBar()->value() - diff);
    }
}

void PreviewArea::zoomSel()
{
    if (image) {
        image->zoomSel();
        setFocus();
        // first move to the bottom-right to get the selection at the top/left of the window
        ensureVisible(image->size().width()-10, image->size().height()-10);
        ensureVisible(image->topLeftX(), image->topLeftY(), SCALE_SELECT_MARGIN, SCALE_SELECT_MARGIN);
    }
}

void PreviewArea::zoom2Fit()
{
    if (image) {
        image->zoom2Fit();
        setFocus();
        ensureVisible(0, 0);
    }
}

void PreviewArea::setTLX(float ratio)
{
    if (image) {
        image->setTLX(ratio);
        image->update();
    }
}

void PreviewArea::setTLY(float ratio)
{
    if (image) {
        image->setTLY(ratio);
        image->update();
    }
}

void PreviewArea::setBRX(float ratio)
{
    if (image) {
        image->setBRX(ratio);
        image->update();
    }
}

void PreviewArea::setBRY(float ratio)
{
    if (image) {
        image->setBRY(ratio);
        image->update();
    }
}

bool PreviewArea::setIconZoomIn(const QIcon &icon)
{
    zoomInAction->setIcon(icon);
    return true;
}

bool PreviewArea::setIconZoomOut(const QIcon &icon)
{
    zoomOutAction->setIcon(icon);
    return true;
}

bool PreviewArea::setIconZoomSel(const QIcon &icon)
{
    zoomSelAction->setIcon(icon);
    return true;
}

bool PreviewArea::setIconZoomFit(const QIcon &icon)
{
    zoom2FitAction->setIcon(icon);
    return true;
}

void PreviewArea::requestVisibility(int tl_x, int tl_y)
{
    setFocus();
    ensureVisible(tl_x, tl_y, 2, 2);
}

}  // NameSpace KSaneIface
