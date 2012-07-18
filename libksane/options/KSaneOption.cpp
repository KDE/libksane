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

// KDE includes
#include <KDebug>
#include <KLocale>

#include "KSaneOptionWidget.h"

KSaneOption::KSaneOption(const SANE_Handle handle, const int index)
    : QObject(), m_handle(handle), m_index(index)
{
    m_data = 0;
    readOption();
}

KSaneOption::~KSaneOption()
{
    if (m_data) {
        free(m_data);
        m_data = 0;
    }
}

void KSaneOption::readOption() 
{
    m_optDesc = sane_get_option_descriptor(m_handle, m_index);
}

KSaneOption::Visibility KSaneOption::visibility()
{
    if (!m_optDesc) return Hidden;

    if (((m_optDesc->cap & SANE_CAP_SOFT_DETECT) == 0) ||
        (m_optDesc->cap & SANE_CAP_INACTIVE) ||
        ((m_optDesc->size == 0) && (optionType(m_optDesc) != TYPE_BUTTON)))
    {
        return Hidden;
    }
    else if ((m_optDesc->cap & SANE_CAP_SOFT_SELECT) == 0) {
        return Disabled;
    }
    return Shown;
}

bool KSaneOption::needsPolling()
{
    if (!m_optDesc) return false;

    if ((m_optDesc->cap & SANE_CAP_SOFT_DETECT) && !(m_optDesc->cap & SANE_CAP_SOFT_SELECT)) {
        kDebug() << saneName() << "optDesc->cap =" << m_optDesc->cap;
        return true;
    }

    return false;
}

const QString KSaneOption::saneName() const
{
    if (m_optDesc == 0) return QString("");
    return QString(m_optDesc->name);
}

const QString KSaneOption::title() const
{
    if (m_optDesc == 0) return QString("");
    return QString(i18n(m_optDesc->title));
}

const QString KSaneOption::description() const
{
    if (m_optDesc == 0) return QString("");
    return QString(i18n(m_optDesc->desc));
}

bool KSaneOption::writeData(void *data)
{
    SANE_Status status;
    SANE_Int res;

    if (visibility() == Disabled) {
        return false;
    }

    status = sane_control_option (m_handle, m_index, SANE_ACTION_SET_VALUE, data, &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << m_optDesc->name << "sane_control_option returned:" << sane_strstatus(status);
        // write failed. re read the current setting
        readValue();
        return false;
    }
    if (res & SANE_INFO_INEXACT) {
        //kDebug() << "write was inexact. Reload value just in case...";
        readValue();
    }

    if (res & SANE_INFO_RELOAD_OPTIONS) {
        emit optsNeedReload();
        // optReload reloads also the values
    }
    else if (res & SANE_INFO_RELOAD_PARAMS) {
        // 'else if' because with optReload we force also valReload :)
        emit valsNeedReload();
    }

    return true;
}


void KSaneOption::readValue() {}

SANE_Word KSaneOption::toSANE_Word(unsigned char *data)
{
    SANE_Word tmp;
    tmp  = (data[0]&0xff);
    tmp += ((SANE_Word)(data[1]&0xff))<<8;
    tmp += ((SANE_Word)(data[2]&0xff))<<16;
    tmp += ((SANE_Word)(data[3]&0xff))<<24;
    return tmp;
}

void KSaneOption::fromSANE_Word(unsigned char *data, SANE_Word from)
{
    data[0] = (from & 0x000000FF);
    data[1] = (from & 0x0000FF00)>>8;
    data[2] = (from & 0x00FF0000)>>16;
    data[3] = (from & 0xFF000000)>>24;
}

qreal KSaneOption::minValue() {return 0.0;}
qreal KSaneOption::maxValue() {return 0.0;}
qreal KSaneOption::value()    {return 0.0;}
const QString KSaneOption::strValue() {return QString();}

bool KSaneOption::setValue(qreal) {return false;}
bool KSaneOption::setStrValue(const QString &) {return false;}

int  KSaneOption::unit() {return m_optDesc->unit;}

bool KSaneOption::storeCurrentData()
{
    SANE_Status status;
    SANE_Int res;

    // check if we can read the value
    if (visibility() == Hidden) return false;

    // read that current value
    if (m_data != 0) free(m_data);
    m_data = (unsigned char *)malloc(m_optDesc->size);
    status = sane_control_option (m_handle, m_index, SANE_ACTION_GET_VALUE, m_data, &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << m_optDesc->name << "sane_control_option returned" << status;
        return false;
    }
    return true;
}

bool KSaneOption::restoreSavedData()
{
    // check if we have saved any data
    if (m_data == 0) return false;

    // check if we can write the value
    if (visibility() == Hidden) return false;
    if (visibility() == Disabled) return false;

    writeData(m_data);
    readValue();
    return true;
}

KSaneOption::KSaneOptType KSaneOption::optionType(const SANE_Option_Descriptor *optDesc)
{
    if (!optDesc) return TYPE_DETECT_FAIL;

    switch (optDesc->constraint_type) {
        case SANE_CONSTRAINT_NONE:
            switch(optDesc->type)
            {
                case SANE_TYPE_BOOL:
                    return TYPE_CHECKBOX;
                case SANE_TYPE_INT:
                    if (optDesc->size == sizeof(SANE_Word)) return TYPE_SLIDER;
                    kDebug() << "Can not handle:"<< optDesc->title;
                    kDebug() << "SANE_CONSTRAINT_NONE && SANE_TYPE_INT";
                    kDebug() << "size" << optDesc->size<< "!= sizeof(SANE_Word)";
                    break;
                case SANE_TYPE_FIXED:
                    if (optDesc->size == sizeof(SANE_Word)) return TYPE_SLIDER_F;
                    kDebug() << "Can not handle:"<< optDesc->title;
                    kDebug() << "SANE_CONSTRAINT_NONE && SANE_TYPE_FIXED";
                    kDebug() << "size" << optDesc->size<< "!= sizeof(SANE_Word)";
                    break;
                case SANE_TYPE_BUTTON:
                    return TYPE_BUTTON;
                case SANE_TYPE_STRING:
                    return TYPE_ENTRY;
                case SANE_TYPE_GROUP:
                    return TYPE_DETECT_FAIL;
            }
            break;
        case SANE_CONSTRAINT_RANGE:
            switch(optDesc->type) {
                case SANE_TYPE_BOOL:
                    return TYPE_CHECKBOX;
                case SANE_TYPE_INT:
                    if (optDesc->size == sizeof(SANE_Word)) return TYPE_SLIDER;
                    
                    if ((strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR) == 0) ||
                        (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_R) == 0) ||
                        (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_G) == 0) ||
                        (strcmp(optDesc->name, SANE_NAME_GAMMA_VECTOR_B) == 0))
                    {
                        return TYPE_GAMMA;
                    }
                    kDebug() << "Can not handle:"<< optDesc->title;
                    kDebug() << "SANE_CONSTRAINT_RANGE && SANE_TYPE_INT && !SANE_NAME_GAMMA_VECTOR...";
                    kDebug() << "size" << optDesc->size<< "!= sizeof(SANE_Word)";
                    break;
                case SANE_TYPE_FIXED:
                    if (optDesc->size == sizeof(SANE_Word)) return TYPE_SLIDER_F;
                    kDebug() << "Can not handle:"<< optDesc->title;
                    kDebug() << "SANE_CONSTRAINT_RANGE && SANE_TYPE_FIXED";
                    kDebug() << "size" << optDesc->size<< "!= sizeof(SANE_Word)";
                    kDebug() << "Analog Gamma vector?";
                    break;
                case SANE_TYPE_STRING:
                    kDebug() << "Can not handle:" << optDesc->title;
                    kDebug() << "SANE_CONSTRAINT_RANGE && SANE_TYPE_STRING";
                    return TYPE_DETECT_FAIL;
                case SANE_TYPE_BUTTON:
                    return TYPE_BUTTON;
                case SANE_TYPE_GROUP:
                    return TYPE_DETECT_FAIL;
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
        case SANE_CONSTRAINT_STRING_LIST:
            return TYPE_COMBO;
    }
    return TYPE_DETECT_FAIL;
}

KLocalizedString KSaneOption::unitString()
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        return KLocalizedString();
        case SANE_UNIT_PIXEL:       return ki18ncp("SpinBox parameter unit", " Pixel", " Pixels");
        case SANE_UNIT_BIT:         return ki18ncp("SpinBox parameter unit", " Bit", " Bits");
        case SANE_UNIT_MM:          return ki18nc("SpinBox parameter unit (Millimeter)", " mm");
        case SANE_UNIT_DPI:         return ki18nc("SpinBox parameter unit (Dots Per Inch)", " DPI");
        case SANE_UNIT_PERCENT:     return ki18nc("SpinBox parameter unit (Percentage)", " %");
        case SANE_UNIT_MICROSECOND: return ki18nc("SpinBox parameter unit (Microseconds)", " µs");
    }
    return KLocalizedString();
}

QString KSaneOption::unitDoubleString()
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        return QString("");
        case SANE_UNIT_PIXEL:       return i18nc("Double numbers. SpinBox parameter unit", " Pixels");
        case SANE_UNIT_BIT:         return i18nc("Double numbers. SpinBox parameter unit", " Bits");
        case SANE_UNIT_MM:          return i18nc("Double numbers. SpinBox parameter unit (Millimeter)", " mm");
        case SANE_UNIT_DPI:         return i18nc("Double numbers. SpinBox parameter unit (Dots Per Inch)", " DPI");
        case SANE_UNIT_PERCENT:     return i18nc("Double numbers. SpinBox parameter unit (Percentage)", " %");
        case SANE_UNIT_MICROSECOND: return i18nc("Double numbers. SpinBox parameter unit (Microseconds)", " µs");
    }
    return QString("");
}

