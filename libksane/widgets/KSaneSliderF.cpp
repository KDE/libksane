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
#include "KSaneSliderF.h"
#include "KSaneSliderF.moc"

#include <KDoubleNumInput>

KSaneSliderF::KSaneSliderF(QWidget *parent, const QString& ltext,
                               qreal min, qreal max, qreal step)
: KSaneOptionWidget(parent, ltext)
{
    m_numInput = new KDoubleNumInput(this);
    m_numInput->setMinimum(min);
    m_numInput->setMaximum(max);
    m_numInput->setValue(min);
    setStep(step);
    m_label->setBuddy(m_numInput);

    m_layout->addWidget(m_numInput, 0, 1);
    m_layout->setColumnStretch(1, 50);
}

KSaneSliderF::~KSaneSliderF()
{
}

void KSaneSliderF::setSuffix(const QString &text)
{
    m_numInput->setSuffix(text);
}

void KSaneSliderF::setRange(qreal min, qreal max)
{
    m_numInput->setRange(min, max);
}

void KSaneSliderF::setStep(qreal step)
{
    m_fstep = step;
    if (m_fstep == 0) {
        m_fstep = 1;
    }
    m_numInput->setSingleStep(m_fstep);

    int decimals=0;
    qreal tmp_step = m_fstep;
    while (tmp_step < 1) {
        tmp_step *= 10;
        decimals++;
        if (decimals > 5) break;
    }
    m_numInput->setDecimals(decimals);
}

void KSaneSliderF::setValue(qreal value)
{
    m_numInput->setValue(value);
}

qreal KSaneSliderF::value() const
{
    return m_numInput->value();
}

