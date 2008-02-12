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

// Local includes.
#include "radio_select.h"
#include "radio_select.moc"

// Qt includes.
#include <QRadioButton>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

// KDE includes.
#include <KLocale>
#include <KDebug>

namespace KSaneIface
{

RadioSelect::RadioSelect(QWidget *parent)
    : KDialog(parent)
{
    setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QVBoxLayout *layout = new QVBoxLayout(page);

    radio_group = new QGroupBox(this);
    layout->addWidget(radio_group, 100);

    QHBoxLayout *btn_layout = new QHBoxLayout;
    layout->addLayout(btn_layout, 0);
}

int RadioSelect::getSelectedIndex(QWidget *parent,
                                  const QString& group_name,
                                  const QStringList& items,
                                  int default_index)
{
    int i;
    if (items.size() == 0) return -2;

    setParent(parent);
    radio_group->setTitle(group_name);

    // Create the RadioButton list
    QList<QRadioButton *> btn_list;
    for (i=0; i < items.size(); i++) {
        btn_list.append(new QRadioButton(items.at(i), radio_group));
    }

    // Add the device list to the layout
    QVBoxLayout *radio_layout = new QVBoxLayout(radio_group);
    for (i=0; i < btn_list.size(); i++) {
        radio_layout->addWidget(btn_list.at(i));
    }

    int radio_index = default_index;
    if (radio_index >= btn_list.size()) radio_index = btn_list.size()-1;
    if (radio_index < 0) radio_index = 0;

    btn_list.at(radio_index)->toggle();

    // show the dialog and get the selection
    if (exec()) {
        // check which one is selected
        for (i = 0; i < btn_list.size(); i++) {
            if (btn_list.at(i)->isChecked()) break;
        }
        if (i == btn_list.size()) {
            kDebug() << "This is a bad index..." << endl;
            return -1;
        }
        return i;
    }

    return -1;
}

}  // NameSpace KSaneIface
