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

#ifndef PREVIEW_IMAGE_H
#define PREVIEW_IMAGE_H

// Qt includes.
#include <QAction>
#include <QFrame>
#include <QImage>

#define SCALE_SELECT_MARGIN 30

namespace KSaneIface
{

enum sel_change
{
    MOVE_NONE,
    MOVE_TOP_LEFT,
    MOVE_TOP_RIGHT,
    MOVE_BOTTOM_LEFT,
    MOVE_BOTTOM_RIGHT,
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_TOP,
    MOVE_BOTTOM,
    MOVE_ALL
};
/**
 *@author Kåre Särs
 */

/**
 * This is the frame that will contain the preview image
 */
class PreviewImage : public QWidget
{
    Q_OBJECT

public:

    /**
    * Create the frame.
    * \param parent parent widget
    */
    PreviewImage(QWidget *parent);
    ~PreviewImage();

    QSize sizeHint() const;

    void clearSelection();
    QImage *getImage() {return &orig_img;}
    void updateScaledImg();
    int topLeftX() {return tl_x;};
    int topLeftY() {return tl_y;};

public Q_SLOTS:

    void setTLX(float percent);
    void setTLY(float percent);
    void setBRX(float percent);
    void setBRY(float percent);

    void zoomIn();
    void zoomOut();
    void zoomSel();
    void zoom2Fit();

protected:

    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

Q_SIGNALS:

    void newSelection(float tl_x, float tl_y, float br_x, float br_y);
    void requestVisibility(int tl_x, int tl_y);

private:

    QImage image;
    QImage orig_img;

    int x1, y1;
    // tl == Top Left, br == Botom Right
    int tl_x, tl_y, br_x, br_y;
    int dtlx, dtly, dbrx, dbry;
    // tl_x_r = tl_x / image.width()
    float tl_x_r, tl_y_r, br_x_r, br_y_r;

    bool lmb;
    bool have_selection;
    sel_change change;
    float scalefactor;
};

}  // NameSpace KSaneIface

#endif // PREVIEW_IMAGE_H
