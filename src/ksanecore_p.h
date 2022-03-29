/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_CORE_PRIVATE_H
#define KSANE_CORE_PRIVATE_H

#include <QTimer>
#include <QTime>
#include <QVector>
#include <QSet>
#include <QList>
#include <QHash>
#include <QVarLengthArray>

#include "ksanecore.h"
#include "ksanebaseoption.h"
#include "ksanescanthread.h"
#include "ksanefinddevicesthread.h"
#include "ksaneauth.h"

/** This namespace collects all methods and classes in LibKSane. */
namespace KSaneIface
{

class KSaneCorePrivate : public QObject
{
    Q_OBJECT

public:
    KSaneCorePrivate(KSaneCore *parent);
    KSaneCore::KSaneOpenStatus loadDeviceOptions();
    void clearDeviceOptions();
    void setDefaultValues();

    void scanIsFinished(KSaneCore::KSaneScanStatus status, const QString &message);

public Q_SLOTS:
    void devicesListUpdated();
    void signalDevicesListUpdate();
    void imageScanFinished();
    void scheduleValuesReload();
    void reloadOptions();
    void reloadValues();

private Q_SLOTS:
    void determineMultiPageScanning(const QVariant &value);
    void setWaitForExternalButton(const QVariant &value);
    void pollPollOptions();
    void batchModeTimerUpdate();

public:

    // device info
    SANE_Handle m_saneHandle = nullptr;
    QString     m_devName;
    QString     m_vendor;
    QString     m_model;

    // Option variables
    QList<KSaneBaseOption *> m_optionsList;
    QList<KSaneOption *>     m_externalOptionsList;
    QHash<KSaneCore::KSaneOptionName, int> m_optionsLocation;
    QList<KSaneBaseOption *> m_optionsPollList;
    QTimer m_readValuesTimer;
    QTimer m_optionPollTimer;
    bool m_optionPollingNaughtylisted = false;

    QString m_saneUserName;
    QString m_sanePassword;

    KSaneScanThread *m_scanThread = nullptr;
    FindSaneDevicesThread *m_findDevThread;
    KSaneAuth             *m_auth;
    KSaneCore             *q;

    // state variables
    // determines whether scanner will send multiple images
    bool m_executeMultiPageScanning = false;
    // scanning has been cancelled externally
    bool m_cancelMultiPageScan = false;
    // next scanning will start with a hardware button press
    bool m_waitForExternalButton = false;
    // batch mode options
    KSaneBaseOption *m_batchMode;
    KSaneBaseOption *m_batchModeDelay;
    QTimer m_batchModeTimer;
    int m_batchModeCounter = 0;
};

}  // NameSpace KSaneIface

#endif // KSANE_CORE_PRIVATE_H
