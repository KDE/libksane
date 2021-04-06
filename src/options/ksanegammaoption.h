/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-31
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_GAMMA_OPTION_H
#define KSANE_GAMMA_OPTION_H

#include "ksaneoption.h"

namespace KSaneIface
{

class KSaneGammaOption : public KSaneOption
{
    Q_OBJECT

public:
    KSaneGammaOption(const SANE_Handle handle, const int index);

    void readValue() override;
    void readOption() override;
    
    QVariant getMaxValue() const override;
    QVariant getValue() const override;
    QString getValueAsString() const override;

public Q_SLOTS:
    bool setValue(const QVariant & value) override;

private:
    void calculateGTwriteData();
    
    int             m_brightness;
    int             m_contrast;
    int             m_gamma;
    QVector<int>    m_gammaTable;
};

}  // NameSpace KSaneIface

#endif // KSANE_GAMMA_OPTION_H
