/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2011 by Kare Sars <kare.sars@iki .fi>
 * Copyright (C) 2014 by Gregor Mitsch: port to KDE5 frameworks
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

// Local includes

#include "labeledentry.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

namespace KSaneIface
{

LabeledEntry::LabeledEntry(QWidget *parent, const QString &ltext)
    : KSaneOptionWidget(parent, ltext)
{
    m_entry = new QLineEdit(this);
    m_reset = new QPushButton(this);
    m_reset->setText(i18nc("Label for button to reset text in a KLineEdit", "Reset"));
    m_set = new QPushButton(this);
    m_set->setText(i18nc("Label for button to write text in a KLineEdit to sane", "Set"));

    m_layout->addWidget(m_entry, 1, 0, 1, 2);
    m_layout->addWidget(m_reset, 1, 2);
    m_layout->addWidget(m_set, 1, 3);
    m_layout->setColumnStretch(1, 50);

    connect(m_reset, &QPushButton::clicked, this, &LabeledEntry::resetClicked);
    connect(m_set, &QPushButton::clicked, this, &LabeledEntry::setClicked);
}

LabeledEntry::~LabeledEntry()
{
}

void LabeledEntry::setText(const QString &text)
{
    m_eText = text;
    m_entry->setText(text);
}

void LabeledEntry::resetClicked()
{
    m_entry->setText(m_eText);
}

void LabeledEntry::setClicked()
{
    m_eText = m_entry->text();
    emit entryEdited(m_eText);
}

}  // NameSpace KSaneIface
