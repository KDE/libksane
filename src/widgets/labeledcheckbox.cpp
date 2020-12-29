/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "labeledcheckbox.h"

namespace KSaneIface
{

LabeledCheckbox::LabeledCheckbox(QWidget *parent, const QString &ltext)
    : KSaneOptionWidget(parent, QString())
{
    chbx = new QCheckBox(ltext, this);
    m_layout->addWidget(chbx, 0, 1);
    m_layout->setColumnStretch(1, 50);

    connect(chbx, &QCheckBox::toggled, this, &LabeledCheckbox::toggled);
}

LabeledCheckbox::~LabeledCheckbox()
{
}

void LabeledCheckbox::setChecked(bool is_checked)
{
    if (is_checked != chbx->isChecked()) {
        chbx->setChecked(is_checked);
    }
}

bool LabeledCheckbox::isChecked()
{
    return chbx->isChecked();
}

}  // NameSpace KSaneIface
