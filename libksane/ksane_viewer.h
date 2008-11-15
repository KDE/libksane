/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2008-11-
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

#ifndef KSANE_VIEWER_H
#define KSANE_VIEWER_H

#include <QGraphicsView>
#include <QWheelEvent>

namespace KSaneIface
{

class KSaneViewer : public QGraphicsView
{
    Q_OBJECT
    public:
        KSaneViewer(QWidget *parent = 0);
        ~KSaneViewer();

        void setQImage(QImage *img);
        void updateImage();

    public Q_SLOTS:
        
        void setTLX(float percent);
        void setTLY(float percent);
        void setBRX(float percent);
        void setBRY(float percent);
        void setSelection(float percent_tlx, float percent_tly, float percent_brx, float percent_bry);
        void clearSelection();
        
        void zoomIn();
        void zoomOut();
        void zoomSel();
        void zoom2Fit();

    Q_SIGNALS:
        void newSelection(float tl_x, float tl_y, float br_x, float br_y);
        
    protected:
        void wheelEvent(QWheelEvent *e);
        void mousePressEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        
    private:
        struct Private;
        Private * const d;

};

}  // NameSpace KSaneIface


#endif


