/* ============================================================
 *
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneOptionPrivate {
    
}
    
KSaneOption::KSaneOption(QObject *parent) : QObject(parent), d(std::unique_ptr<KSaneOptionPrivate>(new KSaneOptionPrivate()))
{
}

KSaneOption::~KSaneOption()
= default;

KSaneOption::KSaneOptionState KSaneOption::state() const
{
    return StateDisabled;
}

QString KSaneOption::name() const
{
    return QString();
}

QString KSaneOption::title() const
{
    return QString();
}

QString KSaneOption::description() const
{
    return QString();
}

KSaneOption::KSaneOptionType KSaneOption::type() const
{  
    return TypeDetectFail;
}

QVariant KSaneOption::minimumValue() const
{  
    return QVariant();
}

QVariant KSaneOption::maximumValue() const
{
    return QVariant();
}

QVariant KSaneOption::stepValue() const
{
    return QVariant();
}

QVariantList KSaneOption::valueList() const
{ 
    return QVariantList();
}

QVariant KSaneOption::value() const
{
    return QVariant();
}

KSaneOption::KSaneOptionUnit KSaneOption::valueUnit() const
{   
    return UnitNone;
}

int KSaneOption::valueSize() const
{
    return 0;
}

bool KSaneOption::setValue(const QVariant &value)
{    
    return false;
}
 
bool KSaneOption::storeCurrentData()
{
    return false;
}

bool KSaneOption::restoreSavedData()
{
    return false;
}

}  // NameSpace KSaneIface
