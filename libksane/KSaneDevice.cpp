/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2011-12-20
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2011 by Kare Sars <kare.sars@iki.fi>
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

#include "KSaneDevice.h"
#include "KSaneDevice.moc"

class KSaneDevicePrivate
{
public:
    KSaneDevice *q;
    KSaneDevicePrivate(KSaneDevice *parent): q(parent) {}
};

KSaneDevice::KSaneDevice(QObject *parent): QObject(parent), d(new KSaneDevicePrivate(this)) {}
KSaneDevice::~KSaneDevice() {}

void KSaneDevice::scanCancel() {}
void KSaneDevice::scanFinal() {}


