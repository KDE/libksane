/* ============================================================
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
    initCheckBox(ltext);
}

LabeledCheckbox::LabeledCheckbox(QWidget *parent, KSaneOption *option)
    : KSaneOptionWidget(parent, option)
{
    initCheckBox(option->title());
    setToolTip(option->description());
    connect(this, &LabeledCheckbox::toggled, option, &KSaneOption::setValue);
    connect(option, &KSaneOption::valueChanged, this, &LabeledCheckbox::setValue);
}

LabeledCheckbox::~LabeledCheckbox()
{
}

void LabeledCheckbox::initCheckBox(const QString &name)
{
    chbx = new QCheckBox(name, this);
    m_layout->addWidget(chbx, 0, 1);
    m_layout->setColumnStretch(1, 50);

    connect(chbx, &QCheckBox::toggled, this, &LabeledCheckbox::toggled);
}

void LabeledCheckbox::setChecked(bool is_checked)
{
    if (is_checked != chbx->isChecked()) {
        chbx->setChecked(is_checked);
    }
}

void LabeledCheckbox::setValue(const QVariant &value)
{
    setChecked(value.toBool());
}

bool LabeledCheckbox::isChecked()
{
    return chbx->isChecked();
}

}  // NameSpace KSaneIface
