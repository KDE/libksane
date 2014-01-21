/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2009 by Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
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

#include "ksane_device_dialog.h"

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QScrollArea>
#include <QLabel>
#include <QDialogButtonBox>

#include <KLocalizedString>

namespace KSaneIface
{

KSaneDeviceDialog::KSaneDeviceDialog(QWidget *parent)
    : QDialog(parent)
{ 
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    
    m_btnGroupDevices = new QButtonGroup(this);

    m_gbDevices = new QGroupBox;
    QVBoxLayout *layout = new QVBoxLayout;
    m_btnContainer = new QWidget;
    m_btnLayout = new QVBoxLayout(m_btnContainer);
    QScrollArea *area = new QScrollArea;
    
    m_gbDevices->setLayout(layout);

    QLabel *explanation =
      new QLabel(i18n("<html>The SANE (Scanner Access Now Easy) system could not find any device.<br>"
                      "Check that the scanner is plugged in and turned on<br>"
                      "or check your systems scanner setup.<br>"
                      "For details about SANE see the "
                      "<a href='http://www.sane-project.org/'>SANE homepage</a>.</html>"));
    explanation->setOpenExternalLinks(true);
    int l,t,r,b;
    layout->getContentsMargins(&l, &t, &r, &b);
    explanation->setContentsMargins(l, t, r, b);

    layout->addWidget(explanation);
    m_gbDevices->adjustSize();  // make sure to see the complete explanation text
    layout->addWidget(area);
    layout->setContentsMargins(0,0,0,0);

    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidget(m_btnContainer);
    
    QDialogButtonBox* bottomButtonBox = new QDialogButtonBox(this);
    bottomButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_btnOk = bottomButtonBox->button(QDialogButtonBox::Ok);
    m_btnReloadDevices = bottomButtonBox->addButton(i18n("Reload devices list"), QDialogButtonBox::ButtonRole::ActionRole);
    layout->addWidget(bottomButtonBox);
    //connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    //connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(bottomButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bottomButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_btnReloadDevices, &QPushButton::clicked, this, &KSaneDeviceDialog::reloadDevicesList);

    topLayout->addWidget(m_gbDevices);
    topLayout->addWidget(bottomButtonBox);
    
    setMinimumHeight(200);
    m_findDevThread = FindSaneDevicesThread::getInstance();

    connect(m_findDevThread, SIGNAL(finished()), this, SLOT(updateDevicesList()));

    reloadDevicesList();
}

KSaneDeviceDialog::~KSaneDeviceDialog() {
    ///@todo wait for thread to finish if its running
}

void KSaneDeviceDialog::reloadDevicesList()
{
    setAvailable(false);
    while (!m_btnGroupDevices->buttons().isEmpty()) {
        delete m_btnGroupDevices->buttons().takeFirst();
    }
    m_gbDevices->setTitle(i18n("Looking for devices. Please wait."));
    m_gbDevices->layout()->itemAt(0)->widget()->hide();  // explanation
    m_btnReloadDevices->setEnabled(true);

    if (!m_findDevThread->isRunning()) {
        m_findDevThread->start();
    }
}

void KSaneDeviceDialog::setAvailable(bool isAvailable)
{
    m_btnOk->setEnabled(isAvailable);
    if (isAvailable) {
        m_selectedDevice = getSelectedName();
        m_btnOk->setFocus();
    }
}

void KSaneDeviceDialog::setDefault(QString defaultBackend)
{
    m_selectedDevice = defaultBackend;
}

QString KSaneDeviceDialog::getSelectedName() {
    QAbstractButton *selectedButton = m_btnGroupDevices->checkedButton();
    if(selectedButton) {
        return selectedButton->objectName();
    }
    return QString();
}

void KSaneDeviceDialog::updateDevicesList()
{
    while (!m_btnGroupDevices->buttons().isEmpty()) {
        delete m_btnGroupDevices->buttons().takeFirst();
    }

    const QList<KSaneWidget::DeviceInfo> list = m_findDevThread->devicesList();
    if (list.isEmpty()) {
        m_gbDevices->setTitle(i18n("Sorry. No devices found."));
        m_gbDevices->layout()->itemAt(0)->widget()->show();  // explanation
        m_gbDevices->layout()->itemAt(1)->widget()->hide();  // scroll area
        m_btnReloadDevices->setEnabled(true);
        return;
    }

    delete m_btnLayout;
    m_btnLayout = new QVBoxLayout;
    m_btnContainer->setLayout(m_btnLayout);
    m_gbDevices->setTitle(i18n("Found devices:"));
    m_gbDevices->layout()->itemAt(0)->widget()->hide();  // explanation
    m_gbDevices->layout()->itemAt(1)->widget()->show();  // scroll area

    for (int i=0; i< list.size(); i++) {
        QRadioButton *b = new QRadioButton(this);
        b->setObjectName(list[i].name);
        b->setToolTip(list[i].name);
        b->setText(QString(QLatin1String("%1 : %2\n%3"))
                    .arg(list[i].vendor)
                    .arg(list[i].model)
                    .arg(list[i].name));

        m_btnLayout->addWidget(b);
        m_btnGroupDevices->addButton(b);
        connect(b, SIGNAL(clicked(bool)), this, SLOT(setAvailable(bool)));
        if((i==0) || (list[i].name == m_selectedDevice)) {
            b->setChecked(true);
            setAvailable(true);
        }
    }

    m_btnLayout->addStretch();

    if (list.size() == 1) {
        m_btnOk->animateClick(); // 2014-01-21: why animated?
    }

    m_btnReloadDevices->setEnabled(true);
}

}
