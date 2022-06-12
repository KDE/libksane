/*
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "ksanedevicedialog.h"

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
    int l, t, r, b;
    layout->getContentsMargins(&l, &t, &r, &b);
    explanation->setContentsMargins(l, t, r, b);

    layout->addWidget(explanation);
    m_gbDevices->adjustSize();  // make sure to see the complete explanation text
    layout->addWidget(area);
    layout->setContentsMargins(0, 0, 0, 0);

    area->setWidgetResizable(true);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidget(m_btnContainer);

    QDialogButtonBox *bottomButtonBox = new QDialogButtonBox(this);
    bottomButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_btnOk = bottomButtonBox->button(QDialogButtonBox::Ok);
    m_btnReloadDevices = bottomButtonBox->addButton(i18n("Reload devices list"), QDialogButtonBox::ButtonRole::ActionRole);
    layout->addWidget(bottomButtonBox);

    connect(bottomButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bottomButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_btnReloadDevices, &QPushButton::clicked, this, &KSaneDeviceDialog::reloadDevicesList);

    topLayout->addWidget(m_gbDevices);
    topLayout->addWidget(bottomButtonBox);

    setMinimumHeight(200);

    reloadDevicesList();
}

KSaneDeviceDialog::~KSaneDeviceDialog()
{
    ///@todo wait for thread to finish if its running
}

void KSaneDeviceDialog::reloadDevicesList()
{
    setAvailable(false);
    qDeleteAll(m_btnGroupDevices->buttons());
    m_gbDevices->setTitle(i18n("Looking for devices. Please wait."));
    m_gbDevices->layout()->itemAt(0)->widget()->hide();  // explanation
    m_btnReloadDevices->setEnabled(false);

    Q_EMIT requestReloadList(KSaneCore::Interface::AllDevices);
}

void KSaneDeviceDialog::setAvailable(bool isAvailable)
{
    m_btnOk->setEnabled(isAvailable);
    if (isAvailable) {
        m_selectedDevice = getSelectedName();
        m_btnOk->setFocus();
    }
}

void KSaneDeviceDialog::setDefault(const QString &defaultBackend)
{
    m_selectedDevice = defaultBackend;
}

QString KSaneDeviceDialog::getSelectedName() const
{
    QAbstractButton *selectedButton = m_btnGroupDevices->checkedButton();
    if (selectedButton) {
        return selectedButton->objectName();
    }
    return QString();
}

void KSaneDeviceDialog::updateDevicesList(const QList<KSaneCore::DeviceInformation*> &list)
{
    qDeleteAll(m_btnGroupDevices->buttons());

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

    for (int i = 0; i < list.size(); ++i) {
        QRadioButton *b = new QRadioButton(this);
        b->setObjectName(list.at(i)->name());
        b->setToolTip(list.at(i)->name());
        b->setText(QStringLiteral("%1 : %2\n%3")
                   .arg(list.at(i)->vendor(), list.at(i)->model(), list.at(i)->name()));

        m_btnLayout->addWidget(b);
        m_btnGroupDevices->addButton(b);
        connect(b, &QRadioButton::clicked, this, &KSaneDeviceDialog::setAvailable);
        if ((i == 0) || (list.at(i)->name() == m_selectedDevice)) {
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
