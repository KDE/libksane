/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_GAMMA_OPTION_H
#define KSANE_GAMMA_OPTION_H

#include "ksanebaseoption.h"

namespace KSaneIface
{

class KSaneGammaOption : public KSaneBaseOption
{
    Q_OBJECT

public:
    KSaneGammaOption(const SANE_Handle handle, const int index);

    void readValue() override;
    void readOption() override;
    
    QVariant maximumValue() const override;
    QVariant value() const override;
    int valueSize() const override;
    QString valueAsString() const override;

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
