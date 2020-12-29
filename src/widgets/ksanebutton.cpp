/* ============================================================
 *
 * This file is part of the KDE project
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
    m_button = new QPushButton(ltext, this);
    m_layout->addWidget(m_button, 0, 1);
    m_layout->addWidget(new QWidget(this), 0, 2);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);

    connect(m_button, &QPushButton::clicked, this, &KSaneButton::clicked);
}

KSaneButton::~KSaneButton()
{
}

}  // NameSpace KSaneIface
