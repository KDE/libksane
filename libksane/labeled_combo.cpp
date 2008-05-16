/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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
#include "labeled_combo.h"
#include "labeled_combo.moc"

// Qt includes.
#include <QLabel>

// KDE includes
#include <KComboBox>
#include <KDebug>

namespace KSaneIface
{

LabeledCombo::LabeledCombo(QWidget *parent, const QString& ltext,
                           const QStringList& list)
            : QFrame(parent)
{
    layout = new QGridLayout(this);
    label  = new QLabel(ltext, this);
    combo  = new KComboBox(this);
    combo->addItems(list);

    label->setBuddy(combo);

    connect(combo, SIGNAL(activated(const QString &)),
            this, SLOT(prActivated(const QString &)));

    connect(combo, SIGNAL(activated(int)),
            this, SLOT(prActivated(int)));

    layout->setSpacing(3);
    layout->setMargin(0);
    layout->addWidget(label, 0, 0, Qt::AlignLeft);
    layout->addWidget(combo, 0, 2);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 50);
    layout->setColumnStretch(2, 0);
}

void LabeledCombo::setCurrentText(const QString &t)
{
    int i;
    for (i=0; i<combo->count(); i++) {
        if (combo->itemText(i) == t) {
            combo->setCurrentIndex(i);
        }
    }
}

bool LabeledCombo::setIcon(const QIcon &icon, const QString& str)
{
    for (int i=0; i < combo->count(); i++) {
        if (combo->itemText(i) == str) {
            combo->setItemIcon(i, icon);
            return true;
        }
    }
    return false;
}

QString LabeledCombo::currentText() const
{
    return(combo->currentText());
}

QString LabeledCombo::text(int i) const
{
    return(combo->itemText(i));
}

void LabeledCombo::setCurrentIndex(int i)
{
    combo->setCurrentIndex(i);
}


void LabeledCombo::widgetSizeHints(int *lab_w, int *cmb_w)
{
    if (lab_w != 0) *lab_w = label->sizeHint().width();
    if (cmb_w != 0) *cmb_w = combo->sizeHint().width();;
}

void LabeledCombo::setColumnWidths(int lab_w, int cmb_w)
{
    //kDebug() << "lab_w =" << lab_w << "cmb_w =" << cmb_w;
    layout->setColumnMinimumWidth(0, lab_w);
    layout->setColumnMinimumWidth(2, cmb_w);
}

int LabeledCombo::count() const
{
    return(combo->count());
}

void LabeledCombo::prActivated(const QString &t)
{
    emit activated(t);
}

void LabeledCombo::prActivated(int i)
{
    emit activated(i);
}

}  // NameSpace KSaneIface
