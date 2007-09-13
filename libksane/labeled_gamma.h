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

#ifndef LABELED_GAMMA_H
#define LABELED_GAMMA_H

// Qt includes.

#include <QFrame>

// Local includes.

#include "libksane_export.h"
#include "labeled_slider.h"
#include "gamma_disp.h"

/**
  *@author Kåre Särs
  */

namespace KSaneIface
{

/**
 * A wrapper for a checkbox
 */
class LIBKSANE_EXPORT LabeledGamma : public QFrame
{
    Q_OBJECT

public:

   /**
    * Create the checkbox.
    *
    * \param parent parent widget
    * \param text is the text describing the checkbox.
    */
    LabeledGamma(QWidget *parent, const QString& text, int elements);
    ~LabeledGamma();

    void setColor(QColor color);
    void setSize(int size);
    const QVector<int> &gammaTablePtr(void) { return gam_tbl; }
    int size(void) {return (int)(max_val+1);}
    void widgetSizeHints(int *lab_w, int *spi_w);
    void setColumnWidhts(int lab_w, int spi_w);

public Q_SLOTS:

    void setValues(int bri, int con, int gam);

private Q_SLOTS:

    void calculateGT(void);

Q_SIGNALS:

    void gammaChanged(int bri, int con, int gam);
    void gammaTableChanged(const QVector<int> &gamma_tbl);

private:

    LabeledSlider *bri_slider;
    LabeledSlider *con_slider;
    LabeledSlider *gam_slider;

    QVector<int>   gam_tbl;
    double         max_val;

    GammaDisp     *gamma_disp;
};

}  // NameSpace KSaneIface

#endif // LABELED_GAMMA_H
