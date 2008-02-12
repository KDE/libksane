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
#include "labeled_entry.h"
#include "labeled_entry.moc"

// Qt includes.
#include <QHBoxLayout>

namespace KSaneIface
{

LabeledEntry::LabeledEntry(QWidget *parent, const QString& ltext)
            : QFrame(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setSpacing(2);
    hb->setMargin(0);
    label = new QLabel(ltext, this);
    entry = new KLineEdit(this);
    reset = new QPushButton(this);
    reset->setText("Reset");
    set = new QPushButton(this);
    set->setText("Set");

    hb->addWidget(label);
    hb->addWidget(entry);
    hb->addWidget(reset);
    hb->addWidget(set);
    hb->activate();

    connect(reset, SIGNAL(clicked()),
            this, SLOT(resetClicked()));

    connect(set, SIGNAL(clicked()),
            this, SLOT(setClicked()));
}

LabeledEntry::~LabeledEntry()
{
}

void LabeledEntry::setText(const QString& text)
{
    e_text = text;
    entry->setText(text);
}

void LabeledEntry::resetClicked()
{
    entry->setText(e_text);
}

void LabeledEntry::setClicked()
{
    e_text = entry->text();
    emit entryEdited(e_text);
}

}  // NameSpace KSaneIface
