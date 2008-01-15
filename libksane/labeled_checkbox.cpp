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

// Qt includes.


// Local includes.

#include "labeled_checkbox.h"
#include "labeled_checkbox.moc"
#include <QHBoxLayout>

namespace KSaneIface
{

LabeledCheckbox::LabeledCheckbox(QWidget *parent, const QString& ltext)
               : QFrame(parent)
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setSpacing(2);
    hb->setMargin(2);
    chbx = new QCheckBox(ltext, this);
    hb->addWidget(chbx);
    hb->activate();

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

void LabeledCheckbox::prToggled(bool on)
{
    emit toggled(on);
}

}  // NameSpace KSaneIface
