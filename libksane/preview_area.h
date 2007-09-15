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

#ifndef PREVIEW_AREA_H
#define PREVIEW_AREA_H

// Qt includes.

#include <QAction>
#include <QFrame>
#include <QImage>
#include <QScrollArea>

// Local includes.

#include "libksane_export.h"
#include "preview_image.h"

/**
 *@author Kåre Särs
 */

namespace KSaneIface
{

/**
 * This is the ScrollArea that will contain the preview image
 */
class LIBKSANE_EXPORT PreviewArea : public QScrollArea
{
    Q_OBJECT

public:

    PreviewArea(QWidget *parent);
    ~PreviewArea();

    QSize sizeHint() const;

    void clearSelection();
    QImage *getImage();
    void updateScaledImg();
    bool setIconFinal(const QIcon &icon);
    bool setIconZoomIn(const QIcon &icon);
    bool setIconZoomOut(const QIcon &icon);
    bool setIconZoomSel(const QIcon &icon);
    bool setIconZoomFit(const QIcon &icon);

public Q_SLOTS:

    void setTLX(float percent);
    void setTLY(float percent);
    void setBRX(float percent);
    void setBRY(float percent);

    void zoomIn();
    void zoomOut();
    void zoomSel();
    void zoom2Fit();

private Q_SLOTS:

    void requestVisibility(int tl_x, int tl_y);

Q_SIGNALS:

    void newSelection(float tl_x, float tl_y, float br_x, float br_y);

private:

    void createContextMenu();

    PreviewImage *image;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *zoomSelAction;
    QAction *zoom2FitAction;
};

}  // NameSpace KSaneIface

#endif // PREVIEW_AREA_H
