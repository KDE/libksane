/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoption.h"

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOption::KSaneOption(const SANE_Handle handle, const int index)
    : QObject(), m_handle(handle), m_index(index)
{
    m_data = nullptr;
    readOption();
    readValue();
}

KSaneOption::~KSaneOption()
{
    if (m_data) {
        free(m_data);
        m_data = nullptr;
    }
}

void KSaneOption::readOption()
{
    m_optDesc = sane_get_option_descriptor(m_handle, m_index);
    Q_EMIT optionReloaded();
}

KSaneOption::KSaneOptionState KSaneOption::state() const
{
    if (!m_optDesc) {
        return StateHidden;
    }

    if (((m_optDesc->cap & SANE_CAP_SOFT_DETECT) == 0) ||
            (m_optDesc->cap & SANE_CAP_INACTIVE) ||
            ((m_optDesc->size == 0) && (type() != TypeAction))) {
        return StateHidden;
    } else if ((m_optDesc->cap & SANE_CAP_SOFT_SELECT) == 0) {
        return StateDisabled;
    }
    return StateActive;
}

bool KSaneOption::needsPolling() const
{
    if (!m_optDesc) {
        return false;
    }

    if ((m_optDesc->cap & SANE_CAP_SOFT_DETECT) && !(m_optDesc->cap & SANE_CAP_SOFT_SELECT)) {
        qCDebug(KSANE_LOG) << name() << "optDesc->cap =" << m_optDesc->cap;
        return true;
    }

    return false;
}

QString KSaneOption::name() const
{
    if (m_optDesc == nullptr) {
        return QString();
    }
    return QString::fromUtf8(m_optDesc->name);
}

QString KSaneOption::title() const
{
    if (m_optDesc == nullptr) {
        return QString();
    }
    return sane_i18n(m_optDesc->title);
}

QString KSaneOption::description() const
{
    if (m_optDesc == nullptr) {
        return QString();
    }
    return sane_i18n(m_optDesc->desc);
}

KSaneOption::KSaneOptionType KSaneOption::type() const
{
    return m_optionType;
}

bool KSaneOption::writeData(void *data)
{
    SANE_Status status;
    SANE_Int res;

    if (state() == StateDisabled) {
        return false;
    }

    status = sane_control_option(m_handle, m_index, SANE_ACTION_SET_VALUE, data, &res);
    if (status != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << m_optDesc->name << "sane_control_option returned:" << sane_strstatus(status);
        // write failed. re read the current setting
        readValue();
        return false;
    }
    if (res & SANE_INFO_INEXACT) {
        //qCDebug(KSANE_LOG) << "write was inexact. Reload value just in case...";
        readValue();
    }

    if (res & SANE_INFO_RELOAD_OPTIONS) {
        Q_EMIT optionsNeedReload();
        // optReload reloads also the values
    } else if (res & SANE_INFO_RELOAD_PARAMS) {
        // 'else if' because with optReload we force also valReload :)
        Q_EMIT valuesNeedReload();
    }

    return true;
}

void KSaneOption::readValue() {}

SANE_Word KSaneOption::toSANE_Word(unsigned char *data)
{
    SANE_Word tmp;
    // if __BYTE_ORDER is not defined we get #if 0 == 0
#if __BYTE_ORDER == __LITTLE_ENDIAN
    tmp  = (data[0] & 0xff);
    tmp += ((SANE_Word)(data[1] & 0xff)) << 8;
    tmp += ((SANE_Word)(data[2] & 0xff)) << 16;
    tmp += ((SANE_Word)(data[3] & 0xff)) << 24;
#else
    tmp  = (data[3] & 0xff);
    tmp += ((SANE_Word)(data[2] & 0xff)) << 8;
    tmp += ((SANE_Word)(data[1] & 0xff)) << 16;
    tmp += ((SANE_Word)(data[0] & 0xff)) << 24;
#endif
    return tmp;
}

void KSaneOption::fromSANE_Word(unsigned char *data, SANE_Word from)
{
    // if __BYTE_ORDER is not defined we get #if 0 == 0
#if __BYTE_ORDER == __LITTLE_ENDIAN
    data[0] = (from & 0x000000FF);
    data[1] = (from & 0x0000FF00) >> 8;
    data[2] = (from & 0x00FF0000) >> 16;
    data[3] = (from & 0xFF000000) >> 24;
#else
    data[3] = (from & 0x000000FF);
    data[2] = (from & 0x0000FF00) >> 8;
    data[1] = (from & 0x00FF0000) >> 16;
    data[0] = (from & 0xFF000000) >> 24;
#endif
}

bool KSaneOption::getMinValue(float &)
{
    return false;
}

bool KSaneOption::getMaxValue(float &)
{
    return false;
}

bool KSaneOption::getStepValue(float &)
{
    return false;
}

QVariantList KSaneOption::getEntryList() const
{
    return QVariantList();
}

bool KSaneOption::getValue(float &)
{
    return false;
}

bool KSaneOption::getValue(QString &)
{
    return false;
}

bool KSaneOption::setValue(const QVariant &)
{
    return false;
}
 
KSaneOption::KSaneOptionUnit KSaneOption::getUnit()
{
    switch (m_optDesc->unit) {
    case SANE_UNIT_PIXEL:       return UnitPixel;
    case SANE_UNIT_BIT:         return UnitBit;
    case SANE_UNIT_MM:          return UnitMilliMeter;
    case SANE_UNIT_DPI:         return UnitDPI;
    case SANE_UNIT_PERCENT:     return UnitPercent;
    case SANE_UNIT_MICROSECOND: return UnitMicroSecond;
    default: return UnitNone;
    }
}

bool KSaneOption::storeCurrentData()
{
    SANE_Status status;
    SANE_Int res;

    // check if we can read the value
    if (state() == StateHidden) {
        return false;
    }

    // read that current value
    if (m_data != nullptr) {
        free(m_data);
    }
    m_data = (unsigned char *)malloc(m_optDesc->size);
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, m_data, &res);
    if (status != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << m_optDesc->name << "sane_control_option returned" << status;
        return false;
    }
    return true;
}

bool KSaneOption::restoreSavedData()
{
    // check if we have saved any data
    if (m_data == nullptr) {
        return false;
    }

    // check if we can write the value
    if (state() == StateHidden) {
        return false;
    }
    if (state() == StateDisabled) {
        return false;
    }

    writeData(m_data);
    readValue();
    return true;
}

KSaneOption::KSaneOptionType KSaneOption::optionType(const SANE_Option_Descriptor *optDesc)
{
    if (!optDesc) {
        return TypeDetectFail;
    }

    switch (optDesc->constraint_type) {
    case SANE_CONSTRAINT_NONE:
        switch (optDesc->type) {
        case SANE_TYPE_BOOL:
            return TypeBool;
        case SANE_TYPE_INT:
            if (optDesc->size == sizeof(SANE_Word)) {
                return TypeInteger;
            }
            qCDebug(KSANE_LOG) << "Can not handle:" << optDesc->title;
            qCDebug(KSANE_LOG) << "SANE_CONSTRAINT_NONE && SANE_TYPE_INT";
            qCDebug(KSANE_LOG) << "size" << optDesc->size << "!= sizeof(SANE_Word)";
            break;
        case SANE_TYPE_FIXED:
            if (optDesc->size == sizeof(SANE_Word)) {
                return TypeFloat;
            }
            qCDebug(KSANE_LOG) << "Can not handle:" << optDesc->title;
            qCDebug(KSANE_LOG) << "SANE_CONSTRAINT_NONE && SANE_TYPE_FIXED";
            qCDebug(KSANE_LOG) << "size" << optDesc->size << "!= sizeof(SANE_Word)";
            break;
        case SANE_TYPE_BUTTON:
            return TypeAction;
        case SANE_TYPE_STRING:
            return TypeString;
        case SANE_TYPE_GROUP:
            return TypeDetectFail;
        }
        break;
    case SANE_CONSTRAINT_RANGE:
        switch (optDesc->type) {
        case SANE_TYPE_BOOL:
            return TypeBool;
        case SANE_TYPE_INT:
            if (optDesc->size == sizeof(SANE_Word)) {
                return TypeInteger;
            }

            if ((strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR) == 0) ||
                    (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_R) == 0) ||
                    (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_G) == 0) ||
                    (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_B) == 0)) {
                return TypeGamma;
            }
            qCDebug(KSANE_LOG) << "Can not handle:" << optDesc->title;
            qCDebug(KSANE_LOG) << "SANE_CONSTRAINT_RANGE && SANE_TYPE_INT && !SANE_NAME_GAMMA_VECTOR...";
            qCDebug(KSANE_LOG) << "size" << optDesc->size << "!= sizeof(SANE_Word)";
            break;
        case SANE_TYPE_FIXED:
            if (optDesc->size == sizeof(SANE_Word)) {
                return TypeFloat;
            }
            qCDebug(KSANE_LOG) << "Can not handle:" << optDesc->title;
            qCDebug(KSANE_LOG) << "SANE_CONSTRAINT_RANGE && SANE_TYPE_FIXED";
            qCDebug(KSANE_LOG) << "size" << optDesc->size << "!= sizeof(SANE_Word)";
            qCDebug(KSANE_LOG) << "Analog Gamma vector?";
            break;
        case SANE_TYPE_STRING:
            qCDebug(KSANE_LOG) << "Can not handle:" << optDesc->title;
            qCDebug(KSANE_LOG) << "SANE_CONSTRAINT_RANGE && SANE_TYPE_STRING";
            return TypeDetectFail;
        case SANE_TYPE_BUTTON:
            return TypeAction;
        case SANE_TYPE_GROUP:
            return TypeDetectFail;
        }
        break;
    case SANE_CONSTRAINT_WORD_LIST:
    case SANE_CONSTRAINT_STRING_LIST:
        return TypeValueList;
    }
    return TypeDetectFail;
}

}  // NameSpace KSaneIface
