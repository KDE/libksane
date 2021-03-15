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

#ifndef KSANE_OPT_SLIDER_H
#define KSANE_OPT_SLIDER_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneOptSlider : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptSlider(const SANE_Handle handle, const int index);

    void readValue() override;

    bool getMinValue(float &min) override;
    bool getMaxValue(float &max) override;
    bool getStepValue(float &step) override;
    bool getValue(float &val) override;
    bool getValue(QString &val) override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private:
    int            m_iVal;
};

}  // NameSpace KSaneIface

#endif
