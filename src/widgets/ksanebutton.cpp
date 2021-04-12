/* ============================================================
 *
 * SPDX-FileCopyrightText: 2008-2011 Kare Sars <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanebutton.h"

namespace KSaneIface
{

KSaneButton::KSaneButton(QWidget *parent, const QString &ltext)
    : KSaneOptionWidget(parent, QString())
{
    initButton(ltext);
}

KSaneButton::KSaneButton(QWidget *parent, KSaneOption *option)
    : KSaneOptionWidget(parent, option)
{
    initButton(option->title());
    setToolTip(option->description());
    connect(this, &KSaneButton::clicked, option, &KSaneOption::setValue);
}

KSaneButton::~KSaneButton()
{
}

void KSaneButton::initButton(const QString &text)
{
    m_button = new QPushButton(text, this);
    m_layout->addWidget(m_button, 0, 1);
    m_layout->addWidget(new QWidget(this), 0, 2);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);

    connect(m_button, &QPushButton::clicked, this, &KSaneButton::clicked);
}

}  // NameSpace KSaneIface
