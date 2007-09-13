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

#ifndef LABELED_CHECKBOX_H
#define LABELED_CHECKBOX_H

// Qt includes.

#include <QFrame>
#include <QCheckBox>

// Local includes.

#include "libksane_export.h"

namespace KSaneIface
{

/**
  *@author Kåre Särs
  */

/**
 * A wrapper for a checkbox
 */
class LIBKSANE_EXPORT LabeledCheckbox : public QFrame
{
    Q_OBJECT

public:

   /**
    * Create the checkbox.
    *
    * \param parent parent widget
    * \param text is the text describing the checkbox.
    */
    LabeledCheckbox(QWidget *parent, const QString& text);
    ~LabeledCheckbox();
    void setChecked(bool);

private Q_SLOTS:

    void prToggled(bool);

Q_SIGNALS:

    void toggled(bool);

private:

    QCheckBox *chbx;
};

}  // NameSpace KSaneIface

#endif // LABELED_CHECKBOX_H
