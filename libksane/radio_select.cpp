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

// Qt includes.

#include <QRadioButton>
#include <QPushButton>
#include <QLayout>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "radio_select.h"
#include "radio_select.moc"

namespace KSaneIface
{

RadioSelect::RadioSelect(QWidget *parent) 
           : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    this->setLayout(layout);

    radio_group = new QGroupBox(this);
    layout->addWidget(radio_group, 100);

    QHBoxLayout *btn_layout = new QHBoxLayout;
    layout->addLayout(btn_layout, 0);

    // add the OK and Cancel buttons to the bottom
    btn_layout->addStretch();
    QPushButton *ok = new QPushButton;
    ok->setText(i18n("OK"));
    btn_layout->addWidget(ok);
    QPushButton *cancel = new QPushButton;
    cancel->setText(i18n("Cancel"));
    btn_layout->addWidget(cancel);

    connect (ok, SIGNAL(clicked()), 
             this, SLOT(accept()));

    connect (cancel, SIGNAL(clicked()), 
             this, SLOT(reject()));
}

int RadioSelect::getSelectedIndex(QWidget *parent,
                                  const QString& group_name,
                                  const QStringList& items,
                                  int default_index)
{
    int i;
    if (items.size() == 0) 
    {
        return -1;
    }

    this->setParent(parent);
    radio_group->setTitle(group_name);

    // Create the RadioButton list
    QList<QRadioButton *> btn_list;
    for (i=0; i < items.size(); i++) 
    {
        //printf("device[i] = %s\n", qPrintable(items.at(i)));
        btn_list.append(new QRadioButton(items.at(i), radio_group));
    }

    // Add the device list to the layout
    QVBoxLayout *radio_layout = new QVBoxLayout(radio_group);
    for (i=0; i < btn_list.size(); i++) 
    {
        radio_layout->addWidget(btn_list.at(i));
    }

    int radio_index = default_index;
    if (radio_index >= btn_list.size()) radio_index = btn_list.size()-1;
    if (radio_index < 0) radio_index = 0;

    btn_list.at(radio_index)->toggle();

    // show the dialog and get the selection
    if (this->exec()) 
    {
        // check which one is selected
        for (i=0; i < btn_list.size(); i++) 
        {
            if (btn_list.at(i)->isChecked()) break;
        }
        if (i == btn_list.size()) 
        {
            kDebug() << "This is a bad index..." << endl;
            return -1;
        }
        return i;
    }
    else 
    {
        return -1;
    }
    return -1;
}

}  // NameSpace KSaneIface
