/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Description : Base class for option widgets
 *
 * SPDX-FileCopyrightText: 2011 Kare Sars <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoptionwidget.h"

// KDE includes

#include <klocalizedstring.h>

namespace KSaneIface
{

KSaneOptionWidget::KSaneOptionWidget(QWidget *parent, const QString &labelText)
    : QWidget(parent)
{
    m_label = new QLabel;
    setLabelText(labelText);

    m_layout = new QGridLayout(this);
    m_layout->addWidget(m_label, 0, 0, Qt::AlignRight);
    m_layout->setColumnStretch(0, 0);
    m_layout->setContentsMargins(0, 0, 0, 0);
}

KSaneOptionWidget::~KSaneOptionWidget()
{
}

void KSaneOptionWidget::setLabelText(const QString &text)
{
    if (text.isEmpty()) {
        m_label->clear();
    } else {
        m_label->setText(i18nc("Label for a scanner option", "%1:", text));
    }
}

int KSaneOptionWidget::labelWidthHint()
{
    return m_label->sizeHint().width();
}

void KSaneOptionWidget::setLabelWidth(int labelWidth)
{
    m_layout->setColumnMinimumWidth(0, labelWidth);
}

}  // NameSpace KSaneIface
