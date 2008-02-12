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
#include "labeled_separator.h"
#include "labeled_separator.moc"

// Qt includes.
#include <QLabel>
#include <QHBoxLayout>

namespace KSaneIface
{

LabeledSeparator::LabeledSeparator(QWidget *parent, const QString& ltext)
                : QFrame( parent )
{
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setSpacing(2);
    hb->setMargin(0);
    QFrame *line1 = new QFrame(this);
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );

    QLabel *label = new QLabel(ltext, this);
    QFrame *line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);

    hb->addWidget(line1,50);
    hb->addWidget(label);
    hb->addWidget(line2,50);
    //hb->activate();
}

LabeledSeparator::~LabeledSeparator()
{
}

}  // NameSpace KSaneIface
