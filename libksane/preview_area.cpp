/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// C++ includes.

#include <stdio.h>
#include <iostream>

// Local includes.

#include "preview_area.h"
#include "preview_area.moc"

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

void PreviewArea::createContextMenu(void)
{
    //Action->setIcon(QIcon(""));
    //Action->setShortcut(tr(""));
    //Action->setStatusTip(tr(""));

    zoomInAction = new QAction(tr("Zoom In"), this);
    connect(zoomInAction, SIGNAL(triggered(void)), image, SLOT(zoomIn(void)));

    zoomOutAction = new QAction(tr("Zoom Out"), this);
    connect(zoomOutAction, SIGNAL(triggered(void)), image, SLOT(zoomOut(void)));

    zoomSelAction = new QAction(tr("Zoom to Selection"), this);
    connect(zoomSelAction, SIGNAL(triggered(void)), image, SLOT(zoomSel(void)));

    zoom2FitAction = new QAction(tr("Zoom to Fit"), this);
    connect(zoom2FitAction, SIGNAL(triggered(void)), image, SLOT(zoom2Fit(void)));

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

QImage *PreviewArea::getImage(void)
{
    if (image) {
        return image->getImage();
    }
    return 0;
}

void PreviewArea::clearSelection(void)
{
    if (image) {
        image->clearSelection();
    }
}

void PreviewArea::updateScaledImg(void)
{
    if (image) {
        image->updateScaledImg();
        image->update();
    }
}

void PreviewArea::zoomIn(void)
{
    if (image) {
        image->zoomIn();
    }
}

void PreviewArea::zoomOut(void)
{
    if (image) {
        image->zoomOut();
    }
}

void PreviewArea::zoomSel(void)
{
    if (image) {
        image->zoomSel();
    }
}

void PreviewArea::zoom2Fit(void)
{
    if (image) {
        image->zoom2Fit();
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


void PreviewArea::requestVisibility(int tl_x, int tl_y)
{
    //printf("requestVisibility %d, %d\n", tl_x, tl_y);
    setFocus();
    // first move to the bottom to get the selection at the top/left of the window
    ensureVisible(2000, 2000);
    ensureVisible(tl_x, tl_y, 20, 20);
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

}  // NameSpace KSaneIface
