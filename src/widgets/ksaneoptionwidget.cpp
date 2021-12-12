/*
 * SPDX-FileCopyrightText: 2011 Kare Sars <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ksaneoptionwidget.h"

// KDE includes

#include <KLocalizedString>

namespace KSaneIface
{

KSaneOptionWidget::KSaneOptionWidget(QWidget *parent, const QString &labelText)
    : QWidget(parent)
{
    m_label = new QLabel(this);
    setLabelText(labelText);
    initWidget();
}

KSaneOptionWidget::KSaneOptionWidget(QWidget *parent, KSane::CoreOption *option)
    : QWidget(parent)
{
    m_option = option;
    m_label = new QLabel;
    connect(option, &KSane::CoreOption::optionReloaded, this, &KSaneOptionWidget::updateVisibility);
    initWidget();
}

KSaneOptionWidget::~KSaneOptionWidget()
{
}

void KSaneOptionWidget::initWidget()
{
    m_layout = new QGridLayout(this);
    m_layout->addWidget(m_label, 0, 0, Qt::AlignRight);
    m_layout->setColumnStretch(0, 0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    updateVisibility();

}

void KSaneOptionWidget::updateVisibility()
{
    if (!m_option) {
        return;
    }

    if (m_option->state() == KSane::CoreOption::StateHidden) {
        hide();
    } else {
        show();
        setEnabled(m_option->state() == KSane::CoreOption::StateActive);
    }
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
