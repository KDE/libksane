/* ============================================================
 *
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoption.h"
#include "ksaneoption_p.h"
#include "ksanebaseoption.h"

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOption::KSaneOption(QObject *parent) : QObject(parent), d(std::unique_ptr<KSaneOptionPrivate>(new KSaneOptionPrivate()))
{
}

KSaneOption::~KSaneOption()
= default;

KSaneOption::KSaneOptionState KSaneOption::state() const
{
    if (d->option != nullptr) {
        return d->option->state();
    } else {
        return StateDisabled;
    }
}

QString KSaneOption::name() const
{
    if (d->option != nullptr) {
        return d->option->name();
    } else {
        return QString();
    }
}

QString KSaneOption::title() const
{
    if (d->option != nullptr) {
        return d->option->title();
    } else {
        return QString();
    }
}

QString KSaneOption::description() const
{
    if (d->option != nullptr) {
        return d->option->description();
    } else {
        return QString();
    }
}

KSaneOption::KSaneOptionType KSaneOption::type() const
{  
    if (d->option != nullptr) {
        return d->option->type();
    } else {
        return TypeDetectFail;
    }
}

QVariant KSaneOption::minimumValue() const
{  
    if (d->option != nullptr) {
        return d->option->minimumValue();
    } else {
        return QVariant();
    }
}

QVariant KSaneOption::maximumValue() const
{
    if (d->option != nullptr) {
        return d->option->maximumValue();
    } else {
        return QVariant();
    }
}

QVariant KSaneOption::stepValue() const
{
    if (d->option != nullptr) {
        return d->option->stepValue();
    } else {
        return QVariant();
    }
}

QVariantList KSaneOption::valueList() const
{ 
    if (d->option != nullptr) {
        return d->option->valueList();
    } else {
        return QVariantList();
    }
}

QVariantList KSaneOption::internalValueList() const
{
    if (d->option != nullptr) {
        return d->option->internalValueList();
    } else {
        return QVariantList();
    }
}

QVariant KSaneOption::value() const
{
    if (d->option != nullptr) {
        return d->option->value();
    } else {
        return QVariant();
    }
}

KSaneOption::KSaneOptionUnit KSaneOption::valueUnit() const
{   
    if (d->option != nullptr) {
        return d->option->valueUnit();
    } else {
        return UnitNone;
    }
}

int KSaneOption::valueSize() const
{
    if (d->option != nullptr) {
        return d->option->valueSize();
    } else {
        return 0;
    }
}

bool KSaneOption::setValue(const QVariant &value)
{    
    if (d->option != nullptr) {
        return d->option->setValue(value);
    } else {
        return false;
    }
}
 
bool KSaneOption::storeCurrentData()
{
    if (d->option != nullptr) {
        return d->option->storeCurrentData();
    } else {
        return false;
    }
}

bool KSaneOption::restoreSavedData()
{
    if (d->option != nullptr) {
        return d->option->restoreSavedData();
    } else {
        return false;
    }
}

}  // NameSpace KSaneIface
