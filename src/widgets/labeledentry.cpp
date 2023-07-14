/*
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

// Local includes

#include "labeledentry.h"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// KDE includes

#include <KLocalizedString>

namespace KSaneIface
{

LabeledEntry::LabeledEntry(QWidget *parent, const QString &ltext)
    : KSaneOptionWidget(parent, ltext)
{
    initEntry();
}

LabeledEntry::LabeledEntry(QWidget *parent, KSaneCore::Option *option)
    : KSaneOptionWidget(parent, option)
{
    initEntry();
    setToolTip(option->description());
    setLabelText(option->title());
    connect(this, &LabeledEntry::entryEdited, option, &KSaneCore::Option::setValue);
    connect(option, &KSaneCore::Option::valueChanged, this, &LabeledEntry::setValue);
    QString value = option->value().toString();
    m_entry->setText(value);
}

LabeledEntry::~LabeledEntry()
{
}

void LabeledEntry::initEntry()
{
    m_entry = new QLineEdit(this);
    m_reset = new QPushButton(this);
    m_reset->setText(i18nc("Label for button to reset text in a KLineEdit", "Reset"));
    m_set = new QPushButton(this);
    m_set->setText(i18nc("Label for button to write text in a KLineEdit to sane", "Set"));

    m_layout->addWidget(m_entry, 1, 0, 1, 2);
    m_layout->addWidget(m_reset, 1, 2);
    m_layout->addWidget(m_set, 1, 3);
    m_layout->setColumnStretch(1, 50);

    connect(m_reset, &QPushButton::clicked, this, &LabeledEntry::resetClicked);
    connect(m_set, &QPushButton::clicked, this, &LabeledEntry::setClicked);
}

void LabeledEntry::setText(const QString &text)
{
    m_eText = text;
    m_entry->setText(text);
}

void LabeledEntry::setValue(const QVariant &value)
{
    const QString text = value.toString();
    if (!text.isEmpty()) {
        setText(text);
    }
}

void LabeledEntry::resetClicked()
{
    m_entry->setText(m_eText);
}

void LabeledEntry::setClicked()
{
    m_eText = m_entry->text();
    Q_EMIT entryEdited(m_eText);
}

}  // NameSpace KSaneIface

#include "moc_labeledentry.cpp"
