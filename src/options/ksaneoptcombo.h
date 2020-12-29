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

#ifndef KSANE_OPT_COMBO_H
#define KSANE_OPT_COMBO_H

#include "ksaneoption.h"

#include <utility>

namespace KSaneIface
{

class LabeledCombo;

class KSaneOptCombo : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptCombo(const SANE_Handle handle, const int index);

    void createWidget(QWidget *parent) override;

    void readValue() override;
    void readOption() override;

    bool getMinValue(float &max) override;
    bool getValue(float &val) override;
    bool setValue(float val) override;
    bool getValue(QString &val) override;
    bool setValue(const QString &val) override;
    bool hasGui() override;

private Q_SLOTS:
    void comboboxChangedIndex(int val);

private:
    QList<std::pair<QString, QString>> genComboStringList() const;
    QString getSaneComboString(int ival) const;
    QString getSaneComboString(float fval) const;
    std::pair<QString, QString> getSaneComboString(unsigned char *data) const;

    LabeledCombo *m_combo;
    QString       m_currentText;
};

}  // NameSpace KSaneIface

#endif
