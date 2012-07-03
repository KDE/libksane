/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-31
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2009 by Kare Sars <kare dot sars at iki dot fi>
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
#include "KSaneOptGamma.h"
#include "KSaneOptGamma.moc"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <KDebug>
#include <KLocale>

KSaneOptGamma::KSaneOptGamma(const SANE_Handle handle, const int index)
: KSaneOption(handle, index)
{
}


void KSaneOptGamma::gammaTableChanged(const QVector<int> &gam_tbl)
{
    QVector<int> copy = gam_tbl;
    writeData(copy.data());
}

void KSaneOptGamma::readValue()
{
    // Unfortunately gamma table to brigthness, contrast and gamma is
    // not easy nor fast.. ergo not done
}


const QString KSaneOptGamma::strValue()
{
    int bri = 0;
    int con = 0;
    int gam = 0;
    // FIXME 
    //m_gamma->getValues(bri, con, gam);
    return QString().sprintf("%d:%d:%d", bri, con, gam);
}

bool KSaneOptGamma::setStrValue(const QString &/*val*/)
{
//    if (!m_gamma) return false;
    if (visibility() == Hidden) return false;

//    m_gamma->setValues(val);
    return true;
}
