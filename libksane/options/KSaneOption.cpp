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
#include "KSaneOption.h"
#include "KSaneOption.moc"

#include "KSaneOptInternal.h"

// KDE includes
#include <KDebug>
#include <KLocale>

KSaneOption::KSaneOption(KSaneOptInternal *option): QObject(option), d(option) {}

KSaneOption::~KSaneOption() {}


KSaneOption::Visibility KSaneOption::visibility() const
{
    return (Visibility)d->visibility();
}

const QString KSaneOption::saneName() const
{
    return d->saneName();
}

const QString KSaneOption::title() const
{
    return d->title();
}

const QString KSaneOption::description() const
{
    return d->description();
}


QVariant KSaneOption::minValue() const
{
    return d->minValue();
}
QVariant KSaneOption::maxValue() const
{
    return d->maxValue();
}

const QStringList KSaneOption::valueList() const
{
    return d->comboStringList();
}

QVariant KSaneOption::value() const
{
    switch (type())
    {
        case Type_CheckBox:
            return (d->value() != 0);
        case Type_Combo:
        case Type_Entry:
        case Type_Gamma:
            return d->strValue();
        case Type_Slider:
        case Type_SliderF:
        case Type_Button:
        case Type_None:
            return d->value();
    }
    return 0.0;
}

bool KSaneOption::setValue(const QVariant &value)
{
    switch (type())
    {
        case Type_CheckBox:
        case Type_Button:
            return d->setValue(value.toBool());
        case Type_Combo:
        case Type_Entry:
        case Type_Gamma:
            return d->setStrValue(value.toString());
        case Type_Slider:
        case Type_SliderF:
        case Type_None:
            return d->setValue(value.toDouble());
    }
    return false;
}

bool KSaneOption::editable() const
{
    return d->editable();
}

KSaneOption::Unit KSaneOption::unit() const
{
    return (Unit)d->unit();
}

KSaneOption::Type KSaneOption::type() const
{
    return (Type)d->optionType();
}


const KLocalizedString KSaneOption::unitString() const
{
    return d->unitString();
}

const QString KSaneOption::unitSpinBoxDoubleString() const
{
    return d->unitSpinBoxDoubleString();
}

