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

#ifndef LABELD_SEPARATOR_H
#define LABELD_SEPARATOR_H

// Qt includes.

#include <QFrame>

/**
  *@author Kåre Särs
  */

namespace KSaneIface
{

/**
 * A separator with a text label
 */
class LabeledSeparator : public QFrame
{
    Q_OBJECT

public:

   /**
    * Create the separator.
    *
    * \param parent parent widget
    * \param text is the text for the separator.
    */
    LabeledSeparator(QWidget *parent, const QString& text);
    ~LabeledSeparator();
};

}  // NameSpace KSaneIface

#endif // LABELD_SEPARATOR_H
