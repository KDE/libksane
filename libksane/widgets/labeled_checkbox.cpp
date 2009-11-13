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

// Local includes.
#include "labeled_checkbox.h"
#include "labeled_checkbox.moc"

// Qt includes
#include <KDebug>

namespace KSaneIface
{

LabeledCheckbox::LabeledCheckbox(QWidget *parent, const QString& ltext)
               : QFrame(parent)
{
    layout = new QGridLayout(this);
    layout->setSpacing(3);
    layout->setMargin(0);
    chbx = new QCheckBox(ltext, this);
    layout->addWidget(chbx, 0, 0);

    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 50);

    connect(chbx, SIGNAL(toggled(bool)),
            this, SLOT(prToggled(bool)));
}

LabeledCheckbox::~LabeledCheckbox()
{
}

void LabeledCheckbox::setChecked(bool is_checked)
{
    if (is_checked != chbx->isChecked()) chbx->setChecked(is_checked);
}

bool LabeledCheckbox::isChecked()
{
    return chbx->isChecked();
}



void LabeledCheckbox::prToggled(bool on)
{
    emit toggled(on);
}

void LabeledCheckbox::widgetSizeHints(int *lab_w, int *rest_w)
{
    if (lab_w != 0) *lab_w = chbx->sizeHint().width();;
    if (rest_w != 0) *rest_w = 0;
}

void LabeledCheckbox::setColumnWidths(int lab_w, int rest_w)
{
    //kDebug() << "lab_w =" << lab_w << "rest_w =" << rest_w;
    layout->setColumnMinimumWidth(0, lab_w);
    layout->setColumnMinimumWidth(1, rest_w);
}

}  // NameSpace KSaneIface
