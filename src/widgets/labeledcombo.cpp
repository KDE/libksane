/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */
#include "labeledcombo.h"

#include <QLabel>

#include <QComboBox>

namespace KSaneIface
{

LabeledCombo::LabeledCombo(QWidget *parent, const QString &ltext, const QStringList &list)
    : KSaneOptionWidget(parent, ltext)
{
    m_combo  = new QComboBox(this);
    m_combo->addItems(list);

    m_label->setBuddy(m_combo);

    connect(m_combo, QOverload<int>::of(&QComboBox::activated), this, &LabeledCombo::activated);

    m_layout->addWidget(m_combo, 0, 1);
    m_layout->addWidget(new QWidget(this), 0, 2);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
}

void LabeledCombo::addItems(const QStringList &list)
{
    m_combo->addItems(list);

    QString tmp;
    for (int i = 0; i < m_combo->count(); ++i) {
        tmp = m_combo->itemText(i);
        m_combo->setItemData(i, tmp, Qt::ToolTipRole);
    }
}

void LabeledCombo::setCurrentText(const QString &t)
{
    for (int i = 0; i < m_combo->count(); ++i) {
        if (m_combo->itemText(i) == t) {
            m_combo->setCurrentIndex(i);
        }
    }
}

QString LabeledCombo::currentText() const
{
    return m_combo->currentText();
}

bool LabeledCombo::setIcon(const QIcon &icon, const QString &str)
{
    for (int i = 0; i < m_combo->count(); ++i) {
        if (m_combo->itemText(i) == str) {
            m_combo->setItemIcon(i, icon);
            return true;
        }
    }
    return false;
}

void LabeledCombo::setCurrentIndex(int i)
{
    m_combo->setCurrentIndex(i);
}
void LabeledCombo::clear()
{
    m_combo->clear();
}

QVariant LabeledCombo::currentData(int role) const
{
    return m_combo->currentData(role);
}

void LabeledCombo::addItem(const QString &text, const QVariant &userData)
{
    m_combo->addItem(text, userData);
}

int LabeledCombo::count() const
{
    return m_combo->count();
}

int LabeledCombo::currentIndex() const
{
    return m_combo->currentIndex();
}


}  // NameSpace KSaneIface
