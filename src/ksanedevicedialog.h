/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_DEVICE_DIALOG_H
#define KSANE_DEVICE_DIALOG_H

#include "ksanewidget.h"

#include <QGroupBox>
#include <QStringList>
#include <QButtonGroup>
#include <QToolTip>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QDialog>

namespace KSaneIface
{

class KSaneDeviceDialog : public QDialog
{
    Q_OBJECT

public:
    KSaneDeviceDialog(QWidget *parent = nullptr);
    ~KSaneDeviceDialog();

    QString getSelectedName() const;
    void setDefault(const QString &);

public Q_SLOTS:
    void updateDevicesList(const QList<KSaneWidget::DeviceInfo> &list);
    void reloadDevicesList();
    
Q_SIGNALS:
    void requestReloadList();

private Q_SLOTS:
    void setAvailable(bool avail);


private:
    QString                 m_defaultBackend;
    QString                 m_selectedDevice;
    QWidget                *m_btnContainer;
    QGroupBox              *m_gbDevices;
    QButtonGroup           *m_btnGroupDevices;
    QVBoxLayout            *m_btnLayout;
    QPushButton            *m_btnReloadDevices;
    QPushButton            *m_btnOk;
};

}

#endif

