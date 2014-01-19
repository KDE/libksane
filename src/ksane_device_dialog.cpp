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

// Local includes
#include "ksane_device_dialog.h"
#include "ksane_device_dialog.moc"


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
    //setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel); // FIXME KF5
    //setButtonText(User1, i18n("Reload devices list")); // FIXME KF5
  
    QDialogButtonBox* buttonbox = new QDialogButtonBox();
    buttonbox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    
    m_btnGroup = new QButtonGroup(this);

    m_btnBox = new QGroupBox;
    QVBoxLayout *layout = new QVBoxLayout;
    m_btnContainer = new QWidget;
    m_btnLayout = new QVBoxLayout(m_btnContainer);
    QScrollArea *area = new QScrollArea;
    
    m_btnBox->setLayout(layout);

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
    m_btnBox->adjustSize();  // make sure to see the complete explanation text
    layout->addWidget(area);
    layout->setContentsMargins(0,0,0,0);

    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidget(m_btnContainer);

    //setMainWidget(m_btnBox); // FIXME KF5
    setMinimumHeight(200);
    m_findDevThread = FindSaneDevicesThread::getInstance();

    connect(m_findDevThread, SIGNAL(finished()), this, SLOT(updateDevicesList()));
    connect(this, SIGNAL(user1Clicked()),        this, SLOT(reloadDevicesList()));

    reloadDevicesList();
}

KSaneDeviceDialog::~KSaneDeviceDialog() {
    ///@todo wait for thread to finish if its running
}

void KSaneDeviceDialog::reloadDevicesList()
{
    setAvailable(false);
    while (!m_btnGroup->buttons().isEmpty()) {
        delete m_btnGroup->buttons().takeFirst();
    }
    m_btnBox->setTitle(i18n("Looking for devices. Please wait."));
    m_btnBox->layout()->itemAt(0)->widget()->hide();  // explanation
    //enableButton(KDialog::User1, false); // FIXME KF5

    if(!m_findDevThread->isRunning()) {
        m_findDevThread->start();
    }
}

void KSaneDeviceDialog::setAvailable(bool avail)
{
    // enableButtonOk(avail); // FIXME KF5
    if(avail) {
        m_selectedDevice = getSelectedName();
        //setButtonFocus(KDialog::Ok); // FIXME KF5
    }
}

void KSaneDeviceDialog::setDefault(QString defaultBackend)
{
        m_selectedDevice = defaultBackend;
}

QString KSaneDeviceDialog::getSelectedName() {
    QAbstractButton *selectedButton = m_btnGroup->checkedButton();
    if(selectedButton) {
        return selectedButton->objectName();
    }
    return QString();
}

void KSaneDeviceDialog::updateDevicesList()
{
    while (!m_btnGroup->buttons().isEmpty()) {
        delete m_btnGroup->buttons().takeFirst();
    }

    const QList<KSaneWidget::DeviceInfo> list = m_findDevThread->devicesList();
    if (list.isEmpty()) {
        m_btnBox->setTitle(i18n("Sorry. No devices found."));
        m_btnBox->layout()->itemAt(0)->widget()->show();  // explanation
        m_btnBox->layout()->itemAt(1)->widget()->hide();  // scroll area
        //enableButton(KDialog::User1, true); // FIXME KF5
        return;
    }

    delete m_btnLayout;
    m_btnLayout = new QVBoxLayout;
    m_btnContainer->setLayout(m_btnLayout);
    m_btnBox->setTitle(i18n("Found devices:"));
    m_btnBox->layout()->itemAt(0)->widget()->hide();  // explanation
    m_btnBox->layout()->itemAt(1)->widget()->show();  // scroll area

    for (int i=0; i< list.size(); i++) {
        QRadioButton *b = new QRadioButton(this);
        b->setObjectName(list[i].name);
        b->setToolTip(list[i].name);
        b->setText(QString(QLatin1String("%1 : %2\n%3"))
                    .arg(list[i].vendor)
                    .arg(list[i].model)
                    .arg(list[i].name));

        m_btnLayout->addWidget(b);
        m_btnGroup->addButton(b);
        connect(b, SIGNAL(clicked(bool)), this, SLOT(setAvailable(bool)));
        if((i==0) || (list[i].name == m_selectedDevice)) {
            b->setChecked(true);
            setAvailable(true);
        }
    }

    m_btnLayout->addStretch();

    if(list.size() == 1) {
        //button(KDialog::Ok)->animateClick(); // FIXME KF5
    }

    //enableButton(KDialog::User1, true); // FIXME KF5
}

}  // NameSpace KSaneIface
