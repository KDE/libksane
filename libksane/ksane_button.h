/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-05-04
 * Description : Sane interface for KDE
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

#ifndef KSANE_BUTTON_H
#define KSANE_BUTTON_H

// Qt includes.
#include <QFrame>
#include <QPushButton>
#include <QGridLayout>

namespace KSaneIface
{

/**
  *@author Kåre Särs
  */

/**
 * A wrapper for a checkbox
 */
class KSaneButton : public QFrame
{
    Q_OBJECT

public:
   /**
    * Create the checkbox.
    *
    * \param parent parent widget
    * \param text is the text describing the checkbox.
    */
    KSaneButton(QWidget *parent, const QString& text);
    ~KSaneButton();
    void widgetSizeHints(int *lab_w, int *chb_w);
    void setColumnWidths(int lab_w, int rest_w);

private Q_SLOTS:
    void btnClicked(bool);

Q_SIGNALS:
    void clicked();

private:
    QPushButton *button;
    QGridLayout *layout;
};

}  // NameSpace KSaneIface

#endif // KSANE_BUTTON_H
