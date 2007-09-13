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

#ifndef RADIO_SELECT_H
#define RADIO_SELECT_H

// Qt includes.

#include <QGroupBox>
#include <QDialog>
#include <QStringList>

// Local includes.

#include "libksane_export.h"

namespace KSaneIface
{

class LIBKSANE_EXPORT RadioSelect : public QDialog
{
    Q_OBJECT

public:

    RadioSelect(QWidget *parent=0);
    ~RadioSelect(void) {};

    int getSelectedIndex(QWidget *parent,
                         const QString& group_name,
                         const QStringList& items,
                         int default_index=0);

private:

    QGroupBox *radio_group;
};

}  // NameSpace KSaneIface

#endif // RADIO_SELECT_H

