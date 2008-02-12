/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
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
#include "sane_option.h"
#include "sane_option.moc"

// Sane includes.
extern "C"
{
#include <sane/saneopts.h>
}


// Qt includes
#include <QtCore/QVarLengthArray>

// KDE includes
#include <KDebug>
#include <KLocale>
#include <kiconloader.h>

#define MIN_FOAT_STEP 0.001

namespace KSaneIface
{

SaneOption::SaneOption(const SANE_Handle s_handle, const int opt_num)
          : sane_handle(s_handle), opt_number(opt_num)
{
    lchebx = 0;
    lcombx = 0;
    frame = 0;
    lslider = 0;
    lfslider = 0;
    lentry = 0;
    sane_data = 0;
    type = SW_DETECT_FAIL;
    iVal=0;
    fVal=0;
    bVal=false;

    cstrl = new QStringList("ComboStringList");

    sane_option = sane_get_option_descriptor(sane_handle, opt_number);
    if (sane_option == 0) {
        kDebug() << "sane_option == 0!!";
        return;
    }
    type = getWidgetType();

    // get the state for the widget
    sw_state = SW_STATE_SHOWN;
    if (((sane_option->cap & SANE_CAP_SOFT_DETECT) == 0) ||
          (sane_option->cap & SANE_CAP_INACTIVE) ||
          (sane_option->size == 0))
    {
        sw_state = SW_STATE_HIDDEN;
    }
    else if ((sane_option->cap & SANE_CAP_SOFT_SELECT) == 0) {
        sw_state = SW_STATE_DISABLED;
    }
    if (type == SW_GROUP) sw_state = SW_STATE_NO_CHANGE;

    readValue();
}

SaneOption::~SaneOption()
{
}

void SaneOption::createWidget(QWidget *parent)
{
    float tmp_step;
    if (sane_option == 0) {
        kDebug() << "sane_option == 0!!";
        return;
    }

    if (frame != 0) delete(frame);

    switch(type)
    {
        case SW_GROUP:
            frame = new LabeledSeparator(parent, i18n(sane_option->title));
            return;
        case SW_CHECKBOX:
            frame = lchebx = new LabeledCheckbox(parent, i18n(sane_option->title));
            connect(lchebx, SIGNAL(toggled(bool)), this, SLOT(checkboxChanged(bool)));
            break;
        case SW_COMBO:
            cstrl = genComboStringList();
            frame = lcombx = new LabeledCombo(parent, i18n(sane_option->title), *cstrl);
            connect(lcombx, SIGNAL(activated(int)), this, SLOT(comboboxChanged(int)));
            break;
        case SW_SLIDER:
            frame = lslider = new LabeledSlider(parent,
                    i18n(sane_option->title),
                    sane_option->constraint.range->min,
                    sane_option->constraint.range->max,
                    sane_option->constraint.range->quant);
            lslider->setSuffix(unitString());
            connect(lslider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
            break;
        case SW_SLIDER_INT:
            frame = lslider = new LabeledSlider(parent,
                    i18n(sane_option->title),
                    SW_INT_MIN,
                    SW_INT_MAX,
                    1);
            lslider->setSuffix(unitString());
            connect(lslider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
            break;
        case SW_F_SLIDER:
            tmp_step = SANE_UNFIX(sane_option->constraint.range->quant);
            if (tmp_step < MIN_FOAT_STEP) tmp_step = MIN_FOAT_STEP;

            frame = lfslider = new LabeledFSlider(parent,
                    i18n(sane_option->title),
                    SANE_UNFIX(sane_option->constraint.range->min),
                    SANE_UNFIX(sane_option->constraint.range->max),
                    tmp_step);
            lfslider->setSuffix(unitString());
            min_change = lfslider->step()/2;
            connect(lfslider, SIGNAL(valueChanged(float)), this, SLOT(fsliderChanged(float)));
            break;
        case SW_F_SLIDER_FIX:

            frame = lfslider = new LabeledFSlider(parent,
                    i18n(sane_option->title),
                    SW_FIXED_MIN,
                    SW_FIXED_MAX ,
                    MIN_FOAT_STEP);
            lfslider->setSuffix(unitString());
            min_change = lfslider->step()/2;
            connect(lfslider, SIGNAL(valueChanged(float)), this, SLOT(fsliderChanged(float)));
            break;
        case SW_ENTRY:
            frame = lentry = new LabeledEntry(parent, i18n(sane_option->title));
            connect(lentry, SIGNAL(entryEdited(const QString&)),
                    this, SLOT(entryChanged(const QString&)));
            break;
        case SW_GAMMA:
            frame = lgamma = new LabeledGamma(parent, i18n(sane_option->title),
                                              sane_option->size/sizeof(SANE_Word));
            connect(lgamma, SIGNAL(gammaTableChanged(const QVector<int> &)),
                    this, SLOT(gammaTableChanged(const QVector<int> &)));
            if (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_R) == 0) lgamma->setColor(Qt::red);
            if (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_G) == 0) lgamma->setColor(Qt::green);
            if (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_B) == 0) lgamma->setColor(Qt::blue);
            break;
        case SW_DETECT_FAIL:
            frame = new LabeledSeparator(parent, ">>> " +
                    QString().sprintf("%d \"", opt_number) +
                            i18n(sane_option->title)+"\" <<<");
            kDebug() << sane_option->title << "SW_DETECT_FAIL";
            break;
    }

    frame->setToolTip(i18n(sane_option->desc));
    
    if (sw_state == SW_STATE_HIDDEN) {
        frame->hide();
    }
    else {
        frame->show();
        frame->setEnabled(sw_state == SW_STATE_SHOWN);
    }
}

SaneOptWidget_t SaneOption::getWidgetType()
{
    switch (sane_option->constraint_type)
    {
        case SANE_CONSTRAINT_NONE:
            switch(sane_option->type)
            {
                case SANE_TYPE_BOOL:
                    return SW_CHECKBOX;
                case SANE_TYPE_INT:
                    if (sane_option->size == sizeof(SANE_Word)) return SW_SLIDER_INT;
                case SANE_TYPE_FIXED:
                    if (sane_option->size == sizeof(SANE_Word)) return SW_F_SLIDER_FIX;
                case SANE_TYPE_BUTTON:
                    return SW_DETECT_FAIL;
                case SANE_TYPE_STRING:
                    return SW_ENTRY;
                case SANE_TYPE_GROUP:
                    return SW_GROUP;
            }
            break;
        case SANE_CONSTRAINT_RANGE:
            switch(sane_option->type)
            {
                case SANE_TYPE_BOOL:
                    return SW_CHECKBOX;
                case SANE_TYPE_INT:
                    if (sane_option->size == sizeof(SANE_Word)) return SW_SLIDER;

                    if ((strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR) == 0) ||
                         (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_R) == 0) ||
                         (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_G) == 0) ||
                         (strcmp(sane_option->name, SANE_NAME_GAMMA_VECTOR_B) == 0))
                    {
                        return SW_GAMMA;
                    }
                    break;
                case SANE_TYPE_FIXED:
                    if (sane_option->size == sizeof(SANE_Word)) return SW_F_SLIDER;
                    break;
                case SANE_TYPE_STRING:
                case SANE_TYPE_BUTTON:
                    return SW_DETECT_FAIL;
                case SANE_TYPE_GROUP:
                    return SW_GROUP;
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
        case SANE_CONSTRAINT_STRING_LIST:
            return SW_COMBO;
    }
    return SW_DETECT_FAIL;
}

QString SaneOption::name()
{
    if (sane_option == 0) return QString("");
    return QString(sane_option->name);
}

QString SaneOption::unitString()
{
    switch(sane_option->unit)
    {
        case SANE_UNIT_NONE:        return QString("");
        case SANE_UNIT_PIXEL:       return i18nc("Parameter unit"," Pixel");
        case SANE_UNIT_BIT:         return i18nc("Parameter unit"," Bit");
        case SANE_UNIT_MM:          return i18nc("Parameter unit"," mm");
        case SANE_UNIT_DPI:         return i18nc("Parameter unit"," DPI");
        case SANE_UNIT_PERCENT:     return QString(" %");
        case SANE_UNIT_MICROSECOND: return i18nc("Parameter unit"," usec");
    }
    return QString("");
}

QStringList *SaneOption::genComboStringList()
{
    int i;

    cstrl->clear();

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            for (i=1; i<=sane_option->constraint.word_list[0]; i++) {
                *cstrl += (QString().sprintf("%d", sane_option->constraint.word_list[i]) +
                        unitString());
            }
            break;
        case SANE_TYPE_FIXED:
            for (i=1; i<=sane_option->constraint.word_list[0]; i++) {
                *cstrl += (QString().sprintf("%f", SANE_UNFIX(sane_option->constraint.word_list[i])) +
                        unitString());
            }
            break;
        case SANE_TYPE_STRING:
            i=0;
            while (sane_option->constraint.string_list[i] != 0) {
                *cstrl += getSaneComboString((unsigned char *)sane_option->constraint.string_list[i]);
                i++;
            }
            break;
        default :
            *cstrl += "NOT HANDELED";
    }
    return cstrl;
}


QString SaneOption::getSaneComboString(unsigned char *data)
{
    QString tmp;
    if (data == 0) return QString();

    if (type != SW_COMBO) {
        return QString();
    }

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            return QString().sprintf("%d", (int)toSANE_Word(data)) + unitString();
        case SANE_TYPE_FIXED:
            return QString().sprintf("%f", SANE_UNFIX(toSANE_Word(data))) + unitString();
        case SANE_TYPE_STRING:
            tmp = i18n(reinterpret_cast<char*>(data));
            // FIXME clean the end of the string !!
            if (tmp.length() > 25) {
                tmp = tmp.left(22);
                tmp += "...";
            }
            tmp += unitString();
            return tmp;
        default :
            break;
    }
    return QString();
}

bool SaneOption::writeData(unsigned char *data)
{
    SANE_Status status;
    SANE_Int res;

    status = sane_control_option (sane_handle, opt_number, SANE_ACTION_SET_VALUE, data, &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << sane_option->name << "sane_control_option returned" << status;
        return false;
    }
    if ((res & SANE_INFO_INEXACT) && (frame != 0)) {
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

void SaneOption::checkboxChanged(bool toggled)
{
    unsigned char data[4];

    fromSANE_Word(data, (toggled) ? 1:0);
    writeData(data);
}

void SaneOption::comboboxChanged(int i)
{
    QVarLengthArray<unsigned char> data(sane_option->size);

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
        case SANE_TYPE_FIXED:
            fromSANE_Word(data.data(), sane_option->constraint.word_list[i+1]);
            break;
        case SANE_TYPE_STRING:
            strncpy(reinterpret_cast<char*>(data.data()), sane_option->constraint.string_list[i], sane_option->size);
            break;
        default:
            kDebug() << "can not handle type:" << sane_option->type;
            break;
    }
    writeData(data.data());
}

bool SaneOption::comboboxChanged(float value)
{
    QVarLengthArray<unsigned char> data(sane_option->size);
    SANE_Word fixed;

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            fromSANE_Word(data.data(), (int)value);
            break;
        case SANE_TYPE_FIXED:
            fixed = SANE_FIX(value);
            fromSANE_Word(data.data(), fixed);
            break;
        default:
            kDebug() << "can only handle SANE_TYPE_INT and SANE_TYPE_FIXED";
            return false;
    }
    writeData(data.data());
    return true;
}

bool SaneOption::comboboxChanged(const QString &value)
{
    QVarLengthArray<unsigned char> data(sane_option->size);
    SANE_Word fixed;
    int i;
    float f;
    bool ok;
    QString tmp;

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            i = value.toInt(&ok);
            if (ok == false) return false;
            fromSANE_Word(data.data(), i);
            break;
        case SANE_TYPE_FIXED:
            f = value.toFloat(&ok);
            if (ok == false) return false;
            fixed = SANE_FIX(f);
            fromSANE_Word(data.data(), fixed);
            break;
        case SANE_TYPE_STRING:
            i = 0;
            while (sane_option->constraint.string_list[i] != 0) {
                tmp = getSaneComboString((unsigned char *)sane_option->constraint.string_list[i]);
                if (value == tmp) {
                    strncpy(reinterpret_cast<char*>(data.data()), sane_option->constraint.string_list[i], sane_option->size);
                    //kDebug() << "->>" << tmp;
                    break;
                }
                i++;
            }
            if (sane_option->constraint.string_list[i] == 0) return false;
            break;
        default:
            kDebug() << "can only handle SANE_TYPE: INT, FIXED and STRING";
            return false;
    }
    writeData(data.data());
    return true;
}

void SaneOption::sliderChanged(int val)
{
    unsigned char data[4];

    if (val == iVal) return;
    iVal = val;
    fromSANE_Word(data, val);
    writeData(data);
}

void SaneOption::fsliderChanged(float val)
{
    unsigned char data[4];
    SANE_Word fixed;

    if (((val-fVal) >= min_change) || ((fVal-val) >= min_change)) {
        //kDebug() <<sane_option->name << fVal << "!=" << val;
        fVal = val;
        fixed = SANE_FIX(val);
        fromSANE_Word(data, fixed);
        writeData(data);
    }
}

void SaneOption::entryChanged(const QString& text)
{
    QVarLengthArray<unsigned char> data(sane_option->size);

    QString tmp;
    tmp += text.left(sane_option->size);
    if (tmp != text) lentry->setText(tmp);
    strcpy(reinterpret_cast<char*>(data.data()), tmp.toLatin1());
    writeData(data.data());
}

void SaneOption::gammaTableChanged(const QVector<int> &gam_tbl)
{
    QVector<int> copy = gam_tbl;
    writeData(reinterpret_cast<unsigned char *>(copy.data()));
}

void SaneOption::readOption()
{
    float tmp_step;
    int tmp_size;
    static const char tmp_binary[] = "Binary";

    sane_option = sane_get_option_descriptor(sane_handle, opt_number);

    // get the state for the widget
    sw_state = SW_STATE_SHOWN;
    if (((sane_option->cap & SANE_CAP_SOFT_DETECT) == 0) ||
          (sane_option->cap & SANE_CAP_INACTIVE) ||
          (sane_option->size == 0))
    {
        sw_state = SW_STATE_HIDDEN;
    }
    else if ((sane_option->cap & SANE_CAP_SOFT_SELECT) == 0) {
        sw_state = SW_STATE_DISABLED;
    }
    if (type == SW_GROUP) {
        sw_state = SW_STATE_NO_CHANGE;
    }

    if (frame == 0) return;

    switch(type)
    {
        case SW_COMBO:
            cstrl = genComboStringList();
            lcombx->clear();
            lcombx->addItems(*cstrl);
            lcombx->setIcon(SmallIcon("color"),
                             getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_COLOR));
            lcombx->setIcon(SmallIcon("gray-scale"),
                             getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_GRAY));
            lcombx->setIcon(SmallIcon("black-white"),
                             getSaneComboString((unsigned char*)SANE_VALUE_SCAN_MODE_LINEART));
            // The epkowa/epson backend uses "Binary" which is the same as "Lineart"
            lcombx->setIcon(SmallIcon("black-white"), i18n(tmp_binary));
            break;
        case SW_SLIDER:
            lslider->setRange(sane_option->constraint.range->min,
                              sane_option->constraint.range->max);
            lslider->setStep(sane_option->constraint.range->quant);
            lslider->setSuffix(unitString());
            break;
        case SW_SLIDER_INT:
            lslider->setRange(SW_INT_MIN, SW_INT_MAX);
            lslider->setStep(1);
            lslider->setSuffix(unitString());
            break;
        case SW_F_SLIDER:
            lfslider->setRange(SANE_UNFIX(sane_option->constraint.range->min),
                               SANE_UNFIX(sane_option->constraint.range->max));

            tmp_step = SANE_UNFIX(sane_option->constraint.range->quant);
            if (tmp_step < MIN_FOAT_STEP) tmp_step = MIN_FOAT_STEP;
            lfslider->setStep(tmp_step);
            min_change = lfslider->step()/2;

            lfslider->setSuffix(unitString());
            break;
        case SW_F_SLIDER_FIX:
            lfslider->setRange(SW_FIXED_MIN, SW_FIXED_MAX);
            lfslider->setStep(MIN_FOAT_STEP);
            min_change = lfslider->step()/2;
            lfslider->setSuffix(unitString());
            break;
        case SW_GAMMA:
            tmp_size = sane_option->size/sizeof(SANE_Word);
            if (lgamma->gammaTablePtr().size() != tmp_size) {
                lgamma->setSize(tmp_size);
            }
            break;
        case SW_GROUP:
        case SW_CHECKBOX:
        case SW_ENTRY:
        case SW_DETECT_FAIL:
            // no changes
            break;
    }

    if (sw_state == SW_STATE_HIDDEN) frame->hide();
    else if (sw_state != SW_STATE_NO_CHANGE) {
        frame->show();
        frame->setEnabled(sw_state == SW_STATE_SHOWN);
    }
}

void SaneOption::readValue()
{
    // check if we can read the value
    if (type == SW_GROUP) return;
    if (sw_state == SW_STATE_HIDDEN) return;

    // read that current value
    QVarLengthArray<unsigned char> data(sane_option->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (sane_handle, opt_number, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    switch(type)
    {
        case SW_GROUP:
            break;
        case SW_CHECKBOX:
            bVal = (toSANE_Word(data.data()) != 0) ? true:false;
            if (lchebx != 0) {
                lchebx->setChecked(bVal);
            }
            emit cbValueRead(bVal);
            break;
        case SW_COMBO:
            if (lcombx != 0) {
                lcombx->setCurrentText(getSaneComboString(data.data()));
            }
            break;
        case SW_SLIDER:
        case SW_SLIDER_INT:
            iVal = toSANE_Word(data.data());
            if ((lslider != 0) &&  (lslider->value() != (int)iVal)) {
                lslider->setValue((int)iVal);
            }
            emit iValueRead((int)iVal);
            break;
        case SW_F_SLIDER:
        case SW_F_SLIDER_FIX:
            fVal = SANE_UNFIX(toSANE_Word(data.data()));
            if (lfslider != 0) {
                if (((lfslider->value() - fVal) >= min_change) ||
                      ((fVal- lfslider->value()) >= min_change) )
                {
                    lfslider->setValue(fVal);
                }
            }
            emit fValueRead(fVal);
            break;
        case SW_ENTRY:
            if (lentry != 0) {
                lentry->setText(reinterpret_cast<char*>(data.data()));
            }
            break;
        case SW_GAMMA:
            // Unfortunately gamma table to brigthness, contrast and gamma is
            // not easy nor fast.. ergo not done
            break;
        case SW_DETECT_FAIL:
            // kDebug() << "Unhandeled";
            break;
    }
}

SANE_Word SaneOption::toSANE_Word(unsigned char *data)
{
    SANE_Word tmp;
    tmp  = (data[0]&0xff);
    tmp += ((SANE_Word)(data[1]&0xff))<<8;
    tmp += ((SANE_Word)(data[2]&0xff))<<16;
    tmp += ((SANE_Word)(data[3]&0xff))<<24;

    return tmp;
}

void SaneOption::fromSANE_Word(unsigned char *data, SANE_Word from)
{
    data[0] = (from & 0x000000FF);
    data[1] = (from & 0x0000FF00)>>8;
    data[2] = (from & 0x00FF0000)>>16;
    data[3] = (from & 0xFF000000)>>24;
}

bool SaneOption::getMaxValue(float *max)
{
    int last;
    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            switch (sane_option->constraint_type)
            {
                case SANE_CONSTRAINT_RANGE:
                    *max = (float)(sane_option->constraint.range->max);
                    return true;
                case SANE_CONSTRAINT_WORD_LIST:
                    last = sane_option->constraint.word_list[0];
                    *max = (float)(sane_option->constraint.word_list[last]);
                    return true;
                case SANE_CONSTRAINT_NONE:
                    if (sane_option->size == sizeof(SANE_Word)) {
                        // FIXME precision is lost.
                        *max = (float)SW_INT_MAX;
                        return true;
                    }
                default:
                    kDebug() << "Constraint must be range or word list!";
            }
            break;
        case SANE_TYPE_FIXED:
            switch (sane_option->constraint_type)
            {
                case SANE_CONSTRAINT_RANGE:
                    *max = SANE_UNFIX(sane_option->constraint.range->max);
                    return true;
                case SANE_CONSTRAINT_WORD_LIST:
                    last = sane_option->constraint.word_list[0];
                    *max = SANE_UNFIX(sane_option->constraint.word_list[last]);
                    return true;
                case SANE_CONSTRAINT_NONE:
                    if (sane_option->size == sizeof(SANE_Word)) {
                        *max = SW_FIXED_MAX;
                        return true;
                    }
                default:
                    kDebug() << "Constraint must be range or word list!";
            }
            break;
        default:
            kDebug() << "type must me INT or FIXED not:" << sane_option->type;
    }
    return false;
}

bool SaneOption::getValue(float *val)
{
    // check if we can read the value
    if (type == SW_GROUP) return false;
    if (sw_state == SW_STATE_HIDDEN) return false;

    // read that current value
    QVarLengthArray<unsigned char> data(sane_option->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (sane_handle, opt_number, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << sane_option->name << "sane_control_option returned" << status;
        return false;
    }

    switch (sane_option->type)
    {
        case SANE_TYPE_INT:
            *val = (float)toSANE_Word(data.data());
            return true;
        case SANE_TYPE_FIXED:
            *val = SANE_UNFIX(toSANE_Word(data.data()));
            return true;
        default:
            kDebug() << "Type" << sane_option->type << "not supported!";
    }
    return false;
}

bool SaneOption::setValue(float value)
{
    //kDebug() << sane_option->name << "set value" << value;
    switch (type)
    {
        case SW_SLIDER:
            sliderChanged((int)(value+0.5));
            if (lslider != 0) {
                lslider->setValue((int)value);
            }
            return true;
        case SW_F_SLIDER:
            fsliderChanged(value);
            if (lfslider != 0) {
                lfslider->setValue(value);
            }
            return true;
        case SW_COMBO:
            if (comboboxChanged(value) == false) {
                return false;
            }
            if (lcombx != 0) {
                // force gui update
                readValue();
            }
            return true;
        default:
            kDebug() << "Only options of type slider, fslider and combo are supported";
    }
    return false;
}

bool SaneOption::setChecked(bool check)
{
    //kDebug() << "=>" << check;
    switch (type)
    {
        case SW_CHECKBOX:
            checkboxChanged(check);
            if (lchebx != 0) {
                readValue();
            }
            return true;
        default:
            kDebug() << "Only works on boolean options";
    }
    return false;
}

bool SaneOption::storeCurrentData()
{
    SANE_Status status;
    SANE_Int res;

    // check if we can read the value
    if (type == SW_GROUP) return false;
    if (sw_state == SW_STATE_HIDDEN) return false;

    // read that current value
    if (sane_data != 0) delete (sane_data);
    sane_data = (unsigned char *)malloc(sane_option->size);
    status = sane_control_option (sane_handle, opt_number, SANE_ACTION_GET_VALUE, sane_data, &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << sane_option->name << "sane_control_option returned" << status;
        return false;
    }
    return true;
}

bool SaneOption::restoreSavedData()
{
    // check if we have saved any data
    if (sane_data == 0) return false;

    // check if we can write the value
    if (type == SW_GROUP) return false;
    if (sw_state == SW_STATE_HIDDEN) return false;
    if (sw_state == SW_STATE_DISABLED) return false;

    writeData(sane_data);

    return true;
}

bool SaneOption::getValue(QString *val)
{
    // check if we can read the value
    if (type == SW_GROUP) return false;
    if (sw_state == SW_STATE_HIDDEN) return false;

    // read that current value
    QVarLengthArray<unsigned char> data(sane_option->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option (sane_handle, opt_number, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << sane_option->name << "sane_control_option returned" << status;
        return false;
    }

    switch(type)
    {
        case SW_CHECKBOX:
            *val = (toSANE_Word(data.data()) != 0) ? QString("true") :QString("false");
            break;
        case SW_COMBO:
            *val = getSaneComboString(data.data());
            break;
        case SW_SLIDER:
            *val = QString().sprintf("%d", (int)toSANE_Word(data.data()));
            break;
        case SW_F_SLIDER:
            *val = QString().sprintf("%f", SANE_UNFIX( toSANE_Word(data.data())));
            break;
        case SW_ENTRY:
            *val = QLatin1String(reinterpret_cast<char*>(data.data()));
            break;
        default:
            kDebug() << sane_option->name << "type:" << type << "is not supported";
            return false;
    }
    return true;
}

bool SaneOption::setValue(const QString &val)
{
    QString tmp;
    bool ok;
    int i;
    float f;

    // check if we can vrite to the value
    if (type == SW_GROUP) return false;
    if (sw_state == SW_STATE_HIDDEN) return false;
    if (sw_state == SW_STATE_DISABLED) return false;

    if (getValue(&tmp) == false) return false;

    if (tmp == val) return true; // no update needed

    switch(type)
    {
        case SW_CHECKBOX:
            checkboxChanged(val == QLatin1String("true"));
            if (lchebx != 0) {
                readValue();
            }
            break;
        case SW_COMBO:
            comboboxChanged(val);
            if (lcombx != 0) {
                readValue();
            }
            break;
        case SW_SLIDER:
            i = val.toInt(&ok);
            if (ok == false) return false;
            sliderChanged(i);
            if (lslider != 0) {
                readValue();
            }
            break;
        case SW_F_SLIDER:
            f = val.toFloat(&ok);
            if (ok == false) return false;
            fsliderChanged(f);
            if (lfslider != 0) {
                readValue();
            }
            break;
        case SW_ENTRY:
            entryChanged(val);
            if (lentry != 0) {
                readValue();
            }
            break;
        default:
            kDebug() << sane_option->name << "type:" << type << "is not supported";
            return false;
    }
    return true;
}


}  // NameSpace KSaneIface
