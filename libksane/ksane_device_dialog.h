/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2009 by Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
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

#ifndef KSANE_DEVICE_DIAG_H
#define KSANE_DEVICE_DIAG_H

#include "ksane_find_devices_thread.h"

// Qt includes.
#include <QGroupBox>
#include <QStringList>
#include <QButtonGroup>
#include <QToolTip>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QBoxLayout>


// KDE includes
#include <KDialog>
#include <klocale.h>

namespace KSaneIface
{

class KSaneDeviceDialog : public KDialog
{
    Q_OBJECT

public:

    KSaneDeviceDialog(QWidget *parent=0);
    ~KSaneDeviceDialog();

    QString getSelectedName();
    void setDefault(QString);

public Q_SLOTS:
    void reloadDevicesList();
    void setAvailable(bool avail);
    void updateDevicesList();

private:
    QWidget                *m_btn_container;
    QGroupBox              *m_btn_box;
    QButtonGroup           *m_btn_group;
    QVBoxLayout            *m_btn_layout;
    QString                 m_default_backend;
    QString                 m_selected_device;
    FindSaneDevicesThread  *m_find_devices_thread;

    bool setDevicesList(const QMap<QString, QString>& list);
    QMap<QString, QString> getDevicesList();
};

}  // NameSpace KSaneIface

#endif // RADIO_SELECT_H
