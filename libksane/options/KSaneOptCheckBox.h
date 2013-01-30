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

#ifndef KSaneOptCheckBox_h
#define KSaneOptCheckBox_h

#include "KSaneOptInternal.h"

class KSaneOptCheckBox : public KSaneOptInternal
{
    Q_OBJECT

public:
    KSaneOptCheckBox(const SANE_Handle handle, const int index);

    void readValue();

    qreal value() const;
    const QString strValue() const;
    bool setValue(qreal val);
    bool setValue(const QString &val);
    bool editable() const {return true;}

private Q_SLOTS:
    void checkBoxChanged(bool toggled);

Q_SIGNALS:
    void buttonPressed(const QString &optionName, const QString &optionLabel, bool pressed);

private:
    bool m_checked;
};

#endif
