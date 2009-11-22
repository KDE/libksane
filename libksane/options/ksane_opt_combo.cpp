/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
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
// Local includes.
#include "ksane_opt_combo.h"
#include "ksane_opt_combo.moc"

#include "labeled_combo.h"

// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <kicon.h>
#include <KDebug>
#include <KLocale>

namespace KSaneIface
{
static const char tmp_binary[] = "Binary";

KSaneOptCombo::KSaneOptCombo(const SANE_Handle handle, const int index)
: KSaneOption(handle, index), m_combo(0)
{
}

void KSaneOptCombo::createWidget(QWidget *parent)
{
    if (m_frame) return;

    m_frame = m_combo = new LabeledCombo(parent, "", QStringList());
    readOption();
    m_frame->setToolTip(i18n(m_optDesc->desc));
    connect(m_combo, SIGNAL(activated(int)), this, SLOT(comboboxChangedIndex(int)));
    readValue();
}

void KSaneOptCombo::widgetSizeHints(int *lab_w, int *rest_w)
{
    if (m_combo) {
        m_combo->widgetSizeHints(lab_w, rest_w);
    }
}

void KSaneOptCombo::setColumnWidths(int lab_w, int rest_w)
{
    if (m_combo) {
        m_combo->setColumnWidths(lab_w, rest_w);
    }
}

void KSaneOptCombo::readValue()
{
    if (state() == STATE_HIDDEN) return;

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    
    m_currentText = getSaneComboString(data.data());
    if (m_combo != 0) {
        m_combo->setCurrentText(m_currentText);
    }
}

void KSaneOptCombo::readOption()
{
    KSaneOption::readOption();
    
    if (!m_combo) return;
    
    m_strList = genComboStringList();
    m_combo->clear();
    m_combo->setLabelText(i18n(m_optDesc->title));
    m_combo->addItems(m_strList);
    m_combo->setIcon(KIcon("color"), getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_COLOR));
    m_combo->setIcon(KIcon("gray-scale"),
                     getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_GRAY));
    m_combo->setIcon(KIcon("black-white"),
                     getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_LINEART));
                     // The epkowa/epson backend uses "Binary" which is the same as "Lineart"
    m_combo->setIcon(KIcon("black-white"), i18n(tmp_binary));
}


QStringList &KSaneOptCombo::genComboStringList()
{
    int i;
    m_strList.clear();
    
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            for (i=1; i<=m_optDesc->constraint.word_list[0]; i++) {
                m_strList += getSaneComboString((int)m_optDesc->constraint.word_list[i]);
            }
            break;
        case SANE_TYPE_FIXED:
            for (i=1; i<=m_optDesc->constraint.word_list[0]; i++) {
                m_strList += getSaneComboString((float)SANE_UNFIX(m_optDesc->constraint.word_list[i]));
            }
            break;
        case SANE_TYPE_STRING:
            i=0;
            while (m_optDesc->constraint.string_list[i] != 0) {
                m_strList += getSaneComboString((unsigned char *)m_optDesc->constraint.string_list[i]);
                i++;
            }
            break;
        default :
            m_strList += "NOT HANDELED";
    }
    return m_strList;
}

QString KSaneOptCombo::getSaneComboString(int ival)
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        break;
        case SANE_UNIT_PIXEL:       return i18np("%1 Pixel","%1 Pixels", ival);
        case SANE_UNIT_BIT:         return i18np("%1 Bit","%1 Bits", ival);
        case SANE_UNIT_MM:          return i18np("%1 mm","%1 mm", ival);
        case SANE_UNIT_DPI:         return i18np("%1 DPI","%1 DPI", ival);
        case SANE_UNIT_PERCENT:     return i18np("%1 %","%1 %", ival);
        case SANE_UNIT_MICROSECOND: return i18np("%1 usec","%1 usec", ival);
    }
    return QString::number(ival);
}

QString KSaneOptCombo::getSaneComboString(float fval)
{
    switch(m_optDesc->unit)
    {
        case SANE_UNIT_NONE:        break;
        case SANE_UNIT_PIXEL:       return i18nc("Parameter and Unit","%1 Pixels", fval);
        case SANE_UNIT_BIT:         return i18nc("Parameter and Unit","%1 Bits", fval);
        case SANE_UNIT_MM:          return i18nc("Parameter and Unit","%1 mm", fval);
        case SANE_UNIT_DPI:         return i18nc("Parameter and Unit","%1 DPI", fval);
        case SANE_UNIT_PERCENT:     return i18nc("Parameter and Unit","%1 %", fval);
        case SANE_UNIT_MICROSECOND: return i18nc("Parameter and Unit","%1 usec", fval);
    }
    return QString::number(fval, 'F', 4);
}

QString KSaneOptCombo::getSaneComboString(unsigned char *data)
{
    QString tmp;
    if (data == 0) return QString();
        
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            return getSaneComboString((int)toSANE_Word(data));
        case SANE_TYPE_FIXED:
            return getSaneComboString((float)SANE_UNFIX(toSANE_Word(data)));
        case SANE_TYPE_STRING:
            tmp = i18n(reinterpret_cast<char*>(data));
            tmp = tmp.simplified();
            if (tmp.length() > 25) {
                tmp = tmp.left(22);
                tmp += "...";
            }
            return tmp;
        default :
            break;
    }
    return QString();
}



void KSaneOptCombo::comboboxChangedIndex(int i)
{
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
            kDebug(51004) << "can not handle type:" << m_optDesc->type;
            return;
    }
    writeData(dataPtr);
    readValue();
}

bool KSaneOptCombo::getMinValue(float &val)
{
    if (state() == STATE_HIDDEN) return false;
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            val = (float)m_optDesc->constraint.word_list[1];
            for (int i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                val = qMin((float)m_optDesc->constraint.word_list[i], val);
            }
            break;
        case SANE_TYPE_FIXED:
            val = (float)SANE_UNFIX(m_optDesc->constraint.word_list[1]);
            for (int i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                val = qMin((float)SANE_UNFIX(m_optDesc->constraint.word_list[i]), val);
            }
            break;
        default:
            kDebug() << "can not handle type:" << m_optDesc->type;
            return false;
    }
    return true;
}

bool KSaneOptCombo::getValue(float &val)
{
    if (state() == STATE_HIDDEN) return false;

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug(51004) << m_optDesc->name << "sane_control_option returned" << status;
        return false;
    }
    
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            val = (float)toSANE_Word(data.data());
            return true;
        case SANE_TYPE_FIXED:
            val = SANE_UNFIX(toSANE_Word(data.data()));
            return true;
        default:
            kDebug(51004) << "Type" << m_optDesc->type << "not supported!";
    }
    return false;
}

bool KSaneOptCombo::setValue(float value)
{
    unsigned char data[4];
    float tmp;
    float minDiff;
    int i;
    int minIndex = 1;
    
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            tmp = (float)m_optDesc->constraint.word_list[minIndex];
            minDiff = qAbs(value - tmp);
            for (i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                tmp = (float)m_optDesc->constraint.word_list[i];
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
            tmp = (float)SANE_UNFIX(m_optDesc->constraint.word_list[minIndex]);
            minDiff = qAbs(value - tmp);
            for (i=2; i<=m_optDesc->constraint.word_list[0]; i++) {
                tmp = (float)SANE_UNFIX(m_optDesc->constraint.word_list[i]);
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
            kDebug(51004) << "can not handle type:" << m_optDesc->type;
            break;
    }
    return false;
}

bool KSaneOptCombo::getValue(QString &val)
{
    if (state() == STATE_HIDDEN) return false;
    val = m_currentText;
    return true;
}

bool KSaneOptCombo::setValue(const QString &val)
{
    if (state() == STATE_HIDDEN) return false;
    if (val == m_currentText) return true;
    
    unsigned char data[4];
    void *data_ptr;
    SANE_Word fixed;
    int i;
    float f;
    bool ok;
    QString tmp;
    
    switch (m_optDesc->type)
    {
        case SANE_TYPE_INT:
            tmp = val.left(val.indexOf(' ')); // strip the unit
            // accept float formating of the string
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
            kDebug(51004) << "can only handle SANE_TYPE: INT, FIXED and STRING";
            return false;
    }
    writeData(data_ptr);

    readValue();
    return true;
}


}  // NameSpace KSaneIface
