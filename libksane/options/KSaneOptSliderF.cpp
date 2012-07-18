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
#include "KSaneOptSliderF.h"
#include "KSaneOptSliderF.moc"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <KDebug>
#include <KLocale>

static const qreal FIXED_MAX = 32767.0;
static const qreal FIXED_MIN =-32768.0;
static const qreal MIN_FIXED_STEP = 0.0001;

KSaneOptSliderF::KSaneOptSliderF(const SANE_Handle handle, const int index)
: KSaneOption(handle, index), m_fVal(0), m_minChange(MIN_FIXED_STEP)
{
}

void KSaneOptSliderF::readValue()
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

    m_fVal = SANE_UNFIX(toSANE_Word(data.data()));
    emit fValueRead(m_fVal);
}


void KSaneOptSliderF::sliderChanged(qreal val)
{
    if (((val-m_fVal) >= m_minChange) || ((m_fVal-val) >= m_minChange)) {
        unsigned char data[4];
        SANE_Word fixed;
        //kDebug() <<m_optDesc->name << fVal << "!=" << val;
        m_fVal = val;
        fixed = SANE_FIX(val);
        fromSANE_Word(data, fixed);
        writeData(data);
    }
}

qreal KSaneOptSliderF::minValue()
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        return SANE_UNFIX(m_optDesc->constraint.range->min);
    }
    return FIXED_MIN;
}

qreal KSaneOptSliderF::maxValue()
{
    if (m_optDesc->constraint_type == SANE_CONSTRAINT_RANGE) {
        return SANE_UNFIX(m_optDesc->constraint.range->max);
    }
    return FIXED_MAX;
}

qreal KSaneOptSliderF::value()
{
    return m_fVal;
}

const QString KSaneOptSliderF::strValue()
{
    return QString::number(m_fVal, 'F', 6);
}

bool KSaneOptSliderF::setValue(qreal val)
{
    if (visibility() == Hidden) return false;
    sliderChanged(val);
    readValue();
    return true;
}

bool KSaneOptSliderF::setValue(const QString &val)
{
    if (visibility() == Hidden) return false;
    sliderChanged(val.toFloat());
    readValue();
    return true;
}
