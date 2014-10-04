/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2009 by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2014 by Gregor Mitsch: port to KDE5 frameworks
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

#include "ksane_opt_button.h"

#include "ksane_button.h"

#include <QDebug>

namespace KSaneIface
{

KSaneOptButton::KSaneOptButton(const SANE_Handle handle, const int index)
: KSaneOption(handle, index), m_button(0)
{
}

void KSaneOptButton::createWidget(QWidget *parent)
{
    if (m_widget) return;

    readOption();

    if (!m_optDesc) {
        qDebug() << "This is a bug";
        m_widget = new KSaneOptionWidget(parent, "");
        return;
    }

    m_widget = m_button = new KSaneButton(parent, i18n(m_optDesc->title));
    m_widget->setToolTip(i18n(m_optDesc->desc));
    connect(m_button, &KSaneButton::clicked, this, &KSaneOptButton::buttonClicked);

    updateVisibility();
    readValue();
}

void KSaneOptButton::buttonClicked()
{
    unsigned char data[4];
    writeData(data);
}

}  // NameSpace KSaneIface
