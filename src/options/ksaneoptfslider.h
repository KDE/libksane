/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_OPT_FSLIDER_H
#define KSANE_OPT_FSLIDER_H

#include "ksaneoption.h"

namespace KSaneIface
{

class LabeledFSlider;

class KSaneOptFSlider : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptFSlider(const SANE_Handle handle, const int index);

    void createWidget(QWidget *parent) override;

    void readValue() override;
    void readOption() override;

    bool getMinValue(float &max) override;
    bool getMaxValue(float &max) override;
    bool getValue(float &val) override;
    bool setValue(float val) override;
    bool getValue(QString &val) override;
    bool setValue(const QString &val) override;
    bool hasGui() override;

Q_SIGNALS:
    void fValueRead(float);

private Q_SLOTS:
    void sliderChanged(float val);

private:
    LabeledFSlider *m_slider;
    float           m_fVal;
    float           m_minChange;
};

}  // NameSpace KSaneIface

#endif
