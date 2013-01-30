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
#include "KSaneOptCombo.h"
#include "KSaneOptCombo.moc"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <kicon.h>
#include <KDebug>
#include <KLocale>

static const char tmp_binary[] = "Binary";

KSaneOptCombo::KSaneOptCombo(const SANE_Handle handle, const int index)
: KSaneOptInternal(handle, index)
{
}

void KSaneOptCombo::readValue()
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

    m_currentText = getSaneComboString(data.data());
}

const QStringList KSaneOptCombo::comboStringList() const
{
    int i;
    QStringList strList;

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            for (i=1; i<=m_optDesc->constraint.word_list[0]; ++i) {
                strList += getSaneComboString((int)m_optDesc->constraint.word_list[i]);
            }
            break;
        case SANE_TYPE_FIXED:
            for (i=1; i<=m_optDesc->constraint.word_list[0]; ++i) {
                strList += getSaneComboString((qreal)SANE_UNFIX(m_optDesc->constraint.word_list[i]));
            }
            break;
        case SANE_TYPE_STRING:
            i=0;
            while (m_optDesc->constraint.string_list[i] != 0) {
                strList += getSaneComboString((unsigned char *)m_optDesc->constraint.string_list[i]);
                i++;
            }
            break;
        default :
            break;
    }
    return strList;
}

const QString KSaneOptCombo::getSaneComboString(int ival) const
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        break;
        case SANE_UNIT_PIXEL:       return i18np("%1 Pixel","%1 Pixels", ival);
        case SANE_UNIT_BIT:         return i18np("%1 Bit","%1 Bits", ival);
        case SANE_UNIT_MM:          return i18np("%1 mm","%1 mm", ival);
        case SANE_UNIT_DPI:         return i18np("%1 DPI","%1 DPI", ival);
        case SANE_UNIT_PERCENT:     return i18np("%1 %","%1 %", ival);
        case SANE_UNIT_MICROSECOND: return i18np("%1 µs","%1 µs", ival);
    }
    return QString::number(ival);
}

const QString KSaneOptCombo::getSaneComboString(qreal fval) const
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        break;
        case SANE_UNIT_PIXEL:       return i18ncp("Parameter and Unit","%1 Pixel", "%1 Pixels", fval);
        case SANE_UNIT_BIT:         return i18ncp("Parameter and Unit","%1 Bit","%1 Bits", fval);
        case SANE_UNIT_MM:          return i18nc("Parameter and Unit (Millimeter)","%1 mm", fval);
        case SANE_UNIT_DPI:         return i18nc("Parameter and Unit (Dots Per Inch)","%1 DPI", fval);
        case SANE_UNIT_PERCENT:     return i18nc("Parameter and Unit (Percentage)","%1 %", fval);
        case SANE_UNIT_MICROSECOND: return i18nc("Parameter and Unit (Microseconds)","%1 µs", fval);
    }
    return QString::number(fval, 'F', 4);
}

const QString KSaneOptCombo::getSaneComboString(unsigned char *data) const
{
    QString tmp;
    if (data == 0) return QString();

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            return getSaneComboString((int)toSANE_Word(data));
        case SANE_TYPE_FIXED:
            return getSaneComboString((qreal)SANE_UNFIX(toSANE_Word(data)));
        case SANE_TYPE_STRING:
            tmp = i18n(reinterpret_cast<char*>(data));
            tmp = tmp.simplified();
            return tmp;
        default :
            break;
    }
    return QString();
}



void KSaneOptCombo::comboboxChangedIndex(int i)
{
//     if (m_combo && (m_combo->currentText() == m_currentText)) {
//         return;
//     }

    unsigned char data[4];
    void *dataPtr;

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
        case SANE_TYPE_FIXED:
            fromSANE_Word(data, m_optDesc->constraint.word_list[i+1]);
            dataPtr = data;
            break;
        case SANE_TYPE_STRING:
            dataPtr = (void *)m_optDesc->constraint.string_list[i];
            break;
        default:
            kDebug() << "can not handle type:" << m_optDesc->type;
            return;
    }
    writeData(dataPtr);
    readValue();
    emit valueChanged();
}

qreal KSaneOptCombo::minValue() const
{
    qreal val=0.0;
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            val = (qreal)m_optDesc->constraint.word_list[1];
            for (int i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                val = qMin((qreal)m_optDesc->constraint.word_list[i], val);
            }
            break;
        case SANE_TYPE_FIXED:
            val = (qreal)SANE_UNFIX(m_optDesc->constraint.word_list[1]);
            for (int i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                val = qMin((qreal)SANE_UNFIX(m_optDesc->constraint.word_list[i]), val);
            }
            break;
        default:
            kDebug() << "can not handle type:" << m_optDesc->type;
    }
    return val;
}

qreal KSaneOptCombo::value() const
{
    // FIXME this function needs checking
    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << m_optDesc->name << "sane_control_option returned" << status;
        return 0.0;
    }

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            return (qreal)toSANE_Word(data.data());
        case SANE_TYPE_FIXED:
            return SANE_UNFIX(toSANE_Word(data.data()));
        default:
            kDebug() << "Type" << m_optDesc->type << "not supported!";
            break;
    }
    return 0.0;
}

const QString KSaneOptCombo::strValue() const
{
    return m_currentText;
}

bool KSaneOptCombo::setValue(qreal value)
{
    unsigned char data[4];
    qreal tmp;
    qreal minDiff;
    int i;
    int minIndex = 1;

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            tmp = (qreal)m_optDesc->constraint.word_list[minIndex];
            minDiff = qAbs(value - tmp);
            for (i=2; i<=m_optDesc->constraint.word_list[0]; ++i) {
                tmp = (qreal)m_optDesc->constraint.word_list[i];
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
            tmp = (qreal)SANE_UNFIX(m_optDesc->constraint.word_list[minIndex]);
            minDiff = qAbs(value - tmp);
            for (i=2; i<=m_optDesc->constraint.word_list[0]; ++i) {
                tmp = (qreal)SANE_UNFIX(m_optDesc->constraint.word_list[i]);
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
            kDebug() << "can not handle type:" << m_optDesc->type;
            break;
    }
    return false;
}

bool KSaneOptCombo::setStrValue(const QString &val)
{
    if (visibility() == Hidden) return false;
    if (val == m_currentText) return true;

    unsigned char data[4];
    void *data_ptr;
    SANE_Word fixed;
    int i;
    qreal f;
    bool ok;
    QString tmp;

    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            tmp = val.left(val.indexOf(' ')); // strip the unit
            // accept qreal formating of the string
            i = (int)(tmp.toFloat(&ok));
            if (ok == false) return false;
            fromSANE_Word(data, i);
            data_ptr = data;
            break;
        case SANE_TYPE_FIXED:
            tmp = val.left(val.indexOf(' ')); // strip the unit
            f = tmp.toFloat(&ok);
            if (ok == false) return false;
            fixed = SANE_FIX(f);
            fromSANE_Word(data, fixed);
            data_ptr = data;
            break;
        case SANE_TYPE_STRING:
            i = 0;
            while (m_optDesc->constraint.string_list[i] != 0) {
                tmp = getSaneComboString((unsigned char *)m_optDesc->constraint.string_list[i]);
                if (val == tmp) {
                    data_ptr = (void *)m_optDesc->constraint.string_list[i];
                    break;
                }
                i++;
            }
            if (m_optDesc->constraint.string_list[i] == 0) return false;
            break;
        default:
            kDebug() << "can only handle SANE_TYPE: INT, FIXED and STRING";
            return false;
    }
    writeData(data_ptr);

    readValue();
    return true;
}

