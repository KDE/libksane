/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanelistoption.h"

#include <QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneListOption::KSaneListOption(const SANE_Handle handle, const int index)
    : KSaneBaseOption(handle, index)
{
    m_optionType = KSaneOption::TypeValueList;
}

void KSaneListOption::readValue()
{
    if (state() == KSaneOption::StateHidden) {
        return;
    }

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    QVariant newValue;
    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        newValue = static_cast<int>(toSANE_Word(data.data()));
        break;
    case SANE_TYPE_FIXED:
        newValue = SANE_UNFIX(toSANE_Word(data.data()));
        break;
    case SANE_TYPE_STRING:
        newValue = sane_i18n(reinterpret_cast<char *>(data.data()));
        break;
    default:
        break;
    }

    if (newValue != m_currentValue) {
        m_currentValue = newValue;
        Q_EMIT valueChanged(m_currentValue);
    }
}

QVariantList KSaneListOption::valueList() const
{
    int i;
    QVariantList list;

    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        for (i = 1; i <= m_optDesc->constraint.word_list[0]; ++i) {
            list << static_cast<int>(m_optDesc->constraint.word_list[i]);;
        }
        break;
    case SANE_TYPE_FIXED:
        for (i = 1; i <= m_optDesc->constraint.word_list[0]; ++i) {
            list << SANE_UNFIX(m_optDesc->constraint.word_list[i]);
        }
        break;
    case SANE_TYPE_STRING:
        i = 0;
        while (m_optDesc->constraint.string_list[i] != nullptr) {
            list << sane_i18n(m_optDesc->constraint.string_list[i]);
            i++;
        }
        break;
    default :
        qCDebug(KSANE_LOG) << "can not handle type:" << m_optDesc->type;
        break;
    }
    return list;
}

QVariantList KSaneListOption::internalValueList() const
{
    int i;
    QVariantList list;

    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        for (i = 1; i <= m_optDesc->constraint.word_list[0]; ++i) {
            list << static_cast<int>(m_optDesc->constraint.word_list[i]);;
        }
        break;
    case SANE_TYPE_FIXED:
        for (i = 1; i <= m_optDesc->constraint.word_list[0]; ++i) {
            list << SANE_UNFIX(m_optDesc->constraint.word_list[i]);
        }
        break;
    case SANE_TYPE_STRING:
        i = 0;
        while (m_optDesc->constraint.string_list[i] != nullptr) {
            list << QString::fromLatin1(m_optDesc->constraint.string_list[i]);
            i++;
        }
        break;
    default :
        qCDebug(KSANE_LOG) << "can not handle type:" << m_optDesc->type;
        break;
    }
    return list;
}

bool KSaneListOption::setValue(const QVariant &value)
{
    bool success = false;
    if (static_cast<QMetaType::Type>(value.type()) == QMetaType::QString) {
        success = setValue(value.toString());
    } else {
        success = setValue(value.toDouble());
    }

    return success;
}

QVariant KSaneListOption::minimumValue() const
{
    QVariant value;
    if (state() == KSaneOption::StateHidden) {
        return value;
    }
    double dValueMin;
    int iValueMin;
    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        iValueMin = static_cast<int>(m_optDesc->constraint.word_list[1]);
        for (int i = 2; i <= m_optDesc->constraint.word_list[0]; i++) {
            iValueMin = qMin(static_cast<int>(m_optDesc->constraint.word_list[i]), iValueMin);
        }
        value = iValueMin;
        break;
    case SANE_TYPE_FIXED:
        dValueMin = SANE_UNFIX(m_optDesc->constraint.word_list[1]);
        for (int i = 2; i <= m_optDesc->constraint.word_list[0]; i++) {
            dValueMin = qMin(SANE_UNFIX(m_optDesc->constraint.word_list[i]), dValueMin);
        }
        value = dValueMin;
        break;
    default:
        qCDebug(KSANE_LOG) << "can not handle type:" << m_optDesc->type;
        return value;
    }
    return value;
}

QVariant KSaneListOption::value() const
{
    if (state() == KSaneOption::StateHidden) {
        return QVariant();
    }
    return m_currentValue;
}

bool KSaneListOption::setValue(double value)
{
    unsigned char data[4];
    double tmp;
    double minDiff;
    int i;
    int minIndex = 1;

    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        tmp = static_cast<double>(m_optDesc->constraint.word_list[minIndex]);
        minDiff = qAbs(value - tmp);
        for (i = 2; i <= m_optDesc->constraint.word_list[0]; ++i) {
            tmp = static_cast<double>(m_optDesc->constraint.word_list[i]);
            if (qAbs(value - tmp) < minDiff) {
                minDiff = qAbs(value - tmp);
                minIndex = i;
            }
        }
        fromSANE_Word(data, m_optDesc->constraint.word_list[minIndex]);
        writeData(data);
        readValue();
        return (minDiff < 1.0);
    case SANE_TYPE_FIXED:
        tmp = SANE_UNFIX(m_optDesc->constraint.word_list[minIndex]);
        minDiff = qAbs(value - tmp);
        for (i = 2; i <= m_optDesc->constraint.word_list[0]; ++i) {
            tmp = SANE_UNFIX(m_optDesc->constraint.word_list[i]);
            if (qAbs(value - tmp) < minDiff) {
                minDiff = qAbs(value - tmp);
                minIndex = i;
            }
        }
        fromSANE_Word(data, m_optDesc->constraint.word_list[minIndex]);
        writeData(data);
        readValue();
        return (minDiff < 1.0);
    default:
        qCDebug(KSANE_LOG) << "can not handle type:" << m_optDesc->type;
        break;
    }
    return false;
}

QString KSaneListOption::valueAsString() const
{
    if (state() == KSaneOption::StateHidden) {
        return QString();
    }
    return m_currentValue.toString();
}

bool KSaneListOption::setValue(const QString &value)
{
    if (state() == KSaneOption::StateHidden) {
        return false;
    }

    unsigned char data[4];
    void* data_ptr = nullptr;
    SANE_Word fixed;
    int i;
    double d;
    bool ok;
    QString tmp;

    switch (m_optDesc->type) {
    case SANE_TYPE_INT:
        i = value.toInt(&ok);
        if (ok) {
            fromSANE_Word(data, i);
            data_ptr = data;  
        } else {
            return false;
        }

        break;
    case SANE_TYPE_FIXED:
        d = value.toDouble(&ok);
        if (ok) {
            fixed = SANE_FIX(d);
            fromSANE_Word(data, fixed);
            data_ptr = data; 
        } else {
            return false;
        }

        break;
    case SANE_TYPE_STRING:
        i = 0;
        while (m_optDesc->constraint.string_list[i] != nullptr) {
            tmp = sane_i18n(m_optDesc->constraint.string_list[i]);
            if (value == tmp) {
                data_ptr = (void *)m_optDesc->constraint.string_list[i];
                break;
            }
            i++;
        }
        if (m_optDesc->constraint.string_list[i] == nullptr) {
            return false;
        }
        break;
    default:
        qCDebug(KSANE_LOG) << "can only handle SANE_TYPE: INT, FIXED and STRING";
        return false;
    }
    writeData(data_ptr);

    readValue();
    return true;
}

}  // NameSpace KSaneIface
