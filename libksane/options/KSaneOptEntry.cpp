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
#include "KSaneOptEntry.h"
#include "KSaneOptEntry.moc"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <KDebug>
#include <KLocale>

KSaneOptEntry::KSaneOptEntry(const SANE_Handle handle, const int index)
: KSaneOptInternal(handle, index)
{
}

void KSaneOptEntry::entryChanged(const QString& text)
{
    QString tmp;
    tmp += text.left(m_optDesc->size);
    if (tmp != text) {
        writeData(tmp.toLatin1().data());
    }
}

void KSaneOptEntry::readValue()
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

    m_string = QString(reinterpret_cast<char*>(data.data()));
}

const QString KSaneOptEntry::strValue() const
{
    return m_string;
}

bool KSaneOptEntry::setStrValue(const QString &val)
{
    if (visibility() == Hidden) return false;
    entryChanged(val);
    readValue();
    return true;
}
