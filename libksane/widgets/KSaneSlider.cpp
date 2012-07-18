/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Copyright (C) 2007-2012 by Kare Sars <kare.sars@iki.fi>
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
#include "KSaneSlider.h"
#include "KSaneSlider.moc"

// KDE includes
#include <KNumInput>

// Qt includes
#include <QLabel>
#include <QSlider>

KSaneSlider::KSaneSlider(QWidget *parent, const QString& ltext,
                             int min, int max, int ste)
: KSaneOptionWidget(parent, ltext)
{
    m_step = ste;
    if (m_step == 0) m_step = 1;

    m_numInput = new KIntNumInput(this);
    m_numInput->setMinimum(min);
    m_numInput->setMaximum(max);
    m_numInput->setSingleStep(m_step);
    m_numInput->setValue(min);
    connect(m_numInput, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
    m_label->setBuddy(m_numInput);

    m_layout->addWidget(m_numInput, 0, 1);
    m_layout->setColumnStretch(1, 50);
}

KSaneSlider::~KSaneSlider()
{
}

void KSaneSlider::setSuffix(const KLocalizedString &text)
{
    m_numInput->setSuffix(text);
}

void KSaneSlider::setValue(int value)
{
    m_numInput->setValue(value);
}

void KSaneSlider::setRange(int min, int max)
{
    m_numInput->setRange(min, max);
}

void KSaneSlider::setStep(int st)
{
    m_step = st;
    if (m_step == 0) m_step = 1;
    m_numInput->setSingleStep(m_step);
}

int KSaneSlider::value() const
{
    return m_numInput->value();
}
