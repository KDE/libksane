/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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

// Local includes.
#include "ksane_device_dialog.h"
#include "ksane_device_dialog.moc"


// Sane includes.
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}


// KDE includes.
#include <KDebug>

namespace KSaneIface
{

KSaneDeviceDialog::KSaneDeviceDialog(QWidget *parent)
    : KDialog(parent)
{

    setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel);
    setButtonText( User1, i18n("Reload devices list") );

    page = new QWidget(this);
    setMainWidget(page);
    QVBoxLayout *layout = new QVBoxLayout(page);

    btn_layout = new QVBoxLayout();
    btn_layout->setDirection(QBoxLayout::TopToBottom);

    btn_box = new QGroupBox(this);
    btn_box->setMinimumHeight(210);
    btn_box->setMinimumWidth(220);
    btn_box->setLayout(btn_layout);

    layout->addWidget(btn_box);

    btn_group = new QButtonGroup(this);

    find_devices_thread = new FindSaneDevicesThread(this);

    connect(find_devices_thread, SIGNAL( finished() ),
            this, SLOT( updateDevicesList() ) );
    setAvailable(false);

    setupActions();

    emit(reloadDevicesList());

}

KSaneDeviceDialog::~KSaneDeviceDialog() {
    ///@todo wait for thread to finish if its running
}

void KSaneDeviceDialog::setupActions()
{
    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(reloadDevicesList()) );
}


void KSaneDeviceDialog::reloadDevicesList()
{
    if(!find_devices_thread->isRunning()) {
        setAvailable(false);
        find_devices_thread->start();
        btn_box->setEnabled(false);
        btn_box->setTitle( i18n("Looking for devices. Please wait.") );
        enableButton(KDialog::User1, false);
    }
}

void KSaneDeviceDialog::updateDevicesList()
{
    QMap<QString,QString> devices_list;
    find_devices_thread->getDevicesList(devices_list);
    setDevicesList( devices_list );
    btn_box->setEnabled(true);
    enableButton(KDialog::User1, true);
}

void KSaneDeviceDialog::setAvailable(bool avail)
{
    enableButtonOk(avail);
    if(avail) {
        m_selected_device = getSelectedName();
        setButtonFocus(KDialog::Ok);
    }
}

void KSaneDeviceDialog::setDefault(QString default_backend)
{
        m_selected_device = default_backend;
}

QString KSaneDeviceDialog::getSelectedName() {
    QAbstractButton *selected_button = btn_group->checkedButton();
    if(selected_button)
        return selected_button->objectName();
    return QString();
}

bool KSaneDeviceDialog::setDevicesList(const QMap<QString, QString>& items)
{
    QRadioButton *b;

    while (!btn_group->buttons().isEmpty()) {
        delete btn_group->buttons().takeFirst();
    }

    if (items.size() == 0) {
        btn_box->setTitle( i18n("Sorry. No devices found.") );
        return false;
    }

    btn_box->setTitle( i18n("Found devices:") );
    QMapIterator<QString, QString> itr(items);
    while (itr.hasNext()) {
        itr.next();
        b = new QRadioButton(itr.value(), this );
        b->setObjectName(itr.key());
        b->setToolTip( itr.key() );
        btn_layout->addWidget(b);
        btn_group->addButton(b);
        connect(b, SIGNAL(clicked(bool)), this, SLOT(setAvailable(bool)) );
        if(itr.key() == m_selected_device) {
            b->setChecked(true);
            setAvailable(true);
        }
    }

    adjustSize();
    return true;
}

}  // NameSpace KSaneIface
