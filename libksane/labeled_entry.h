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

#ifndef LABELED_ENTRY_H
#define LABELED_ENTRY_H

// Qt includes.

#include <QFrame>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

// Local includes.

#include "libksane_export.h"

/**
 *@author Kåre Särs
 */

namespace KSaneIface
{

/**
 * A text entry field with a set and reset button
 */
class LIBKSANE_EXPORT LabeledEntry : public QFrame
{
    Q_OBJECT

public:

   /**
    * Create the entry.
    *
    * \param parent parent widget
    * \param text is the text describing the entry.
    */
    LabeledEntry(QWidget *parent, const QString& text);
    ~LabeledEntry();

    void setText(const QString& text);

private Q_SLOTS:

    void setClicked(void);
    void resetClicked(void);

Q_SIGNALS:

    void entryEdited(const QString& text);

private:

    QLabel *label;
    QLineEdit *entry;
    QPushButton *set;
    QPushButton *reset;
    QString e_text;
};

}  // NameSpace KSaneIface

#endif // LABELED_ENTRY_H
