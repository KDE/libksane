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

#ifndef KSANE_OPT_ENTRY_H
#define KSANE_OPT_ENTRY_H

#include "ksaneoption.h"

namespace KSaneIface
{

class LabeledEntry;

class KSaneOptEntry : public KSaneOption
{
    Q_OBJECT

public:
    KSaneOptEntry(const SANE_Handle handle, const int index);

    void createWidget(QWidget *parent) override;

    void readValue() override;

    bool getValue(float &val) override;
    bool setValue(float val) override;
    bool getValue(QString &val) override;
    bool setValue(const QString &val) override;
    bool hasGui() override;

private Q_SLOTS:
    void entryChanged(const QString &text);

private:
    LabeledEntry *m_entry;
    QString       m_string;
};

}  // NameSpace KSaneIface

#endif
