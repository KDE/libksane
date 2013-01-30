/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Copyright (C) 2009-2012 by Kare Sars <kare.sars@iki.fi>
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
#include "KSaneOptSlider.h"
#include "KSaneOptSlider.moc"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <KDebug>
#include <KLocale>

static const int KSW_INT_MAX = 2147483647;
static const int KSW_INT_MIN = -2147483647-1;  // prevent warning


KSaneOptSlider::KSaneOptSlider(const SANE_Handle handle, const int index)
: KSaneOptInternal(handle, index)
{
}

void KSaneOptSlider::readValue()
{
    if (visibility() == Hidden) return;

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    m_iVal = toSANE_Word(data.data());
// //     if ((m_slider != 0) &&  (m_slider->value() != m_iVal)) {
// //         m_slider->setValue(m_iVal);
// //     }
    emit fValueRead((qreal)m_iVal);
}

void KSaneOptSlider::sliderChanged(int val)
{
    if (val == m_iVal) return;
    unsigned char data[4];
    m_iVal = val;
    fromSANE_Word(data, val);
    writeData(data);
}

qreal KSaneOptSlider::minValue()  const
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        return (qreal)m_optDesc->constraint.range->min;
    }
    return (qreal)KSW_INT_MIN;
}

qreal KSaneOptSlider::maxValue() const
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
       return (qreal)m_optDesc->constraint.range->max;
    }
    return (qreal)KSW_INT_MAX;
}

qreal KSaneOptSlider::value() const
{
    //kDebug() << m_iVal;
    return (qreal)m_iVal;
}

const QString KSaneOptSlider::strValue() const
{
    return QString::number(m_iVal);
}

bool KSaneOptSlider::setValue(qreal val)
{
    if (visibility() == Hidden) return false;
    sliderChanged((int)val);
    readValue();
    return true;
}

bool KSaneOptSlider::setStrValue(const QString &val)
{
    if (visibility() == Hidden) return false;
    sliderChanged(val.toInt());
    readValue();
    return true;
}

