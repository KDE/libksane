/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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

#ifndef SANE_OPTIONS_H
#define SANE_OPTIONS_H

#define SW_INT_MAX (2147483647)
#define SW_INT_MIN (-2147483647-1)

#define SW_FIXED_MAX (32767.0)
#define SW_FIXED_MIN (-32768.0)

// Qt includes.

#include <QFrame>
#include <QCheckBox>

// Sane includes.

extern "C"
{
#include <sane/sane.h>
}

// Local includes.
#include "labeled_separator.h"
#include "labeled_slider.h"
#include "labeled_fslider.h"
#include "labeled_combo.h"
#include "labeled_checkbox.h"
#include "labeled_entry.h"
#include "labeled_gamma.h"

namespace KSaneIface
{

typedef enum
{
    SW_DETECT_FAIL,
    SW_GROUP,
    SW_CHECKBOX,
    SW_SLIDER,
    SW_SLIDER_INT,
    SW_F_SLIDER,
    SW_F_SLIDER_FIX,
    SW_COMBO,
    SW_ENTRY,
    SW_GAMMA
} SaneOptWidget_t;

typedef enum
{
    SW_STATE_HIDDEN,
    SW_STATE_DISABLED,
    SW_STATE_SHOWN,
    SW_STATE_NO_CHANGE
} SaneOptWState;

class SaneOption : public QObject
{
    Q_OBJECT

public:

    SaneOption(const SANE_Handle, const int);
    ~SaneOption();

    void createWidget(QWidget *parent);

    void readOption();
    void readValue();

    QFrame *widget() {return frame;}
    SaneOptWidget_t sw_type() {return type;}
    QString name();
    bool getMaxValue(float *max);
    bool getValue(float *val);
    bool setValue(float percent);
    bool getValue(QString *val);
    bool setValue(const QString &val);
    bool setChecked(bool check);
    bool storeCurrentData();
    bool restoreSavedData();

    LabeledGamma *lgamma;
    LabeledCombo *lcombx;
    LabeledSlider *lslider;
    LabeledFSlider *lfslider;

Q_SIGNALS:

    void optsNeedReload();
    void valsNeedReload();
    void fValueRead(float val);
    void iValueRead(int val);
    void cbValueRead(bool val);

private Q_SLOTS:

    void comboboxChanged(int i);
    void checkboxChanged(bool toggled);
    bool comboboxChanged(const QString &value);
    void sliderChanged(int val);
    void fsliderChanged(float val);
    void entryChanged(const QString& text);
    void gammaTableChanged(const QVector<int> &gam_tbl);

private:

    SaneOptWidget_t getWidgetType();
    QStringList *genComboStringList();
    QString getSaneComboString(unsigned char *data);
    QString getSaneComboString(int ival);
    QString getSaneComboString(float fval);
    SANE_Word toSANE_Word(unsigned char *data);
    void fromSANE_Word(unsigned char *data, SANE_Word from);
    QString unitString();
    bool writeData(unsigned char *data);
    bool comboboxChanged(float value);

private:

    // gui object variables
    QFrame *frame;
    LabeledCheckbox *lchebx;
    QStringList *cstrl;
    LabeledEntry *lentry;

    SaneOptWidget_t type;
    SaneOptWState sw_state;

    // Sane variables
    SANE_Handle sane_handle;
    int opt_number;
    const SANE_Option_Descriptor *sane_option;
    unsigned char *sane_data;
    int iVal;
    float fVal;
    float min_change;
    bool bVal;
};

}  // NameSpace KSaneIface

#endif // SANE_OPTIONS_H
