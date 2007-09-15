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

#ifndef GAMMA_DISP_H
#define GAMMA_DISP_H

// Qt includes.

#include <QWidget>

// Local includes.

#include "libksane_export.h"

/**
 *@author Kåre Särs
 *
 * This is the widget that displays the gamma table.
 */

namespace KSaneIface
{

class LIBKSANE_EXPORT GammaDisp : public QWidget
{
    Q_OBJECT

public:

    /**
    * Create a gamma display.
    * \param parent parent widget
    */
    GammaDisp(QWidget *parent, QVector<int> *tbl);
    ~GammaDisp() {};

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setColor(QColor color);

protected:

    void paintEvent (QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:

    QVector<int> *gam_tbl;
    QColor gam_color;
};

}  // NameSpace KSaneIface

#endif // GAMMA_DISP_H

