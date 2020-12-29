/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoptbutton.h"

#include "ksanebutton.h"

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOptButton::KSaneOptButton(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_button(nullptr)
{
}

void KSaneOptButton::createWidget(QWidget *parent)
{
    if (m_widget) {
        return;
    }

    readOption();

    if (!m_optDesc) {
        qCDebug(KSANE_LOG) << "This is a bug";
        m_widget = new KSaneOptionWidget(parent, QString());
        return;
    }

    m_widget = m_button = new KSaneButton(parent, sane_i18n(m_optDesc->title));
    m_widget->setToolTip(sane_i18n(m_optDesc->desc));
    connect(m_button, &KSaneButton::clicked, this, &KSaneOptButton::buttonClicked);

    updateVisibility();
    readValue();
}

bool KSaneOptButton::hasGui()
{
    return true;
}

void KSaneOptButton::buttonClicked()
{
    unsigned char data[4];
    writeData(data);
}

}  // NameSpace KSaneIface
