/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2009 Matthias Nagl <matthias at nagl dot info>
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

//Qt includes

#include <QMutex>

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include "ksanecore.h"
#include "ksanecore_p.h"

#include <ksane_debug.h>

namespace KSaneIface
{
static int s_objectCount = 0;

Q_GLOBAL_STATIC(QMutex, s_objectMutex)

KSaneCore::KSaneCore(QObject *parent)
    : QObject(parent), d(std::unique_ptr<KSaneCorePrivate>(new KSaneCorePrivate(this)))
{
    SANE_Int    version;
    SANE_Status status;

    s_objectMutex->lock();
    s_objectCount++;

    if (s_objectCount == 1) {
        // only call sane init for the first instance
        status = sane_init(&version, &KSaneAuth::authorization);
        if (status != SANE_STATUS_GOOD) {
            qCDebug(KSANE_LOG) << "libksane: sane_init() failed("
                     << sane_strstatus(status) << ")";
        }
    }
    s_objectMutex->unlock();

    // read the device list to get a list of vendor and model info
    d->m_findDevThread->start();

    d->m_readValuesTimer.setSingleShot(true);
    connect(&d->m_readValuesTimer, &QTimer::timeout, d.get(), &KSaneCorePrivate::reloadValues);
}

KSaneCore::~KSaneCore()
{
    closeDevice();

    s_objectMutex->lock();
    s_objectCount--;
    if (s_objectCount <= 0) {
        // only delete the find-devices and authorization singletons and call sane_exit
        // if this is the last instance
        delete d->m_findDevThread;
        delete d->m_auth;
        sane_exit();
    }
    s_objectMutex->unlock();
}

QString KSaneCore::deviceName() const
{
    return d->m_devName;
}

QString KSaneCore::deviceVendor() const
{
    return d->m_vendor;
}

QString KSaneCore::deviceModel() const
{
    return d->m_model;
}

bool KSaneCore::reloadDevicesList()
{
    /* On some SANE backends, the handle becomes invalid when
     * querying for new devices. Hence, this is only allowed when
     * no device is currently opened. */
    if (d->m_saneHandle == nullptr) {
        d->m_findDevThread->start();
        return true;
    }
    return false;
}

KSaneCore::KSaneOpenStatus KSaneCore::openDevice(const QString &deviceName)
{
    SANE_Status                    status;

    if (d->m_saneHandle != nullptr) {
        // this KSaneCore already has an open device
        return KSaneOpenStatus::OpeningFailed;
    }

    // don't bother trying to open if the device string is empty
    if (deviceName.isEmpty()) {
        return KSaneOpenStatus::OpeningFailed;
    }
    // save the device name
    d->m_devName = deviceName;

    // Try to open the device
    status = sane_open(deviceName.toLatin1().constData(), &d->m_saneHandle);

    if (status == SANE_STATUS_ACCESS_DENIED) {
        return KSaneOpenStatus::OpeningDenied;
    }
 
    if (status != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_open(\"" << deviceName << "\", &handle) failed! status = " << sane_strstatus(status);
        d->m_devName.clear();
        return KSaneOpenStatus::OpeningFailed;
    }

    return d->loadDeviceOptions();
}

KSaneCore::KSaneOpenStatus KSaneCore::openRestrictedDevice(const QString &deviceName, QString userName, QString password)
{
    SANE_Status                    status;

    if (d->m_saneHandle != nullptr) {
        // this KSaneCore already has an open device
        return KSaneOpenStatus::OpeningFailed;
    }

    // don't bother trying to open if the device string is empty
    if (deviceName.isEmpty()) {
        return KSaneOpenStatus::OpeningFailed;
    }
    // save the device name
    d->m_devName = deviceName;
    
    // add/update the device user-name and password for authentication
    d->m_auth->setDeviceAuth(d->m_devName, userName, password);
    
    // Try to open the device
    status = sane_open(deviceName.toLatin1().constData(), &d->m_saneHandle);

    if (status == SANE_STATUS_ACCESS_DENIED) {
        return KSaneOpenStatus::OpeningDenied;
    }

    if (status != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_open(\"" << deviceName << "\", &handle) failed! status = " << sane_strstatus(status);
        d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_devName.clear();
        return KSaneOpenStatus::OpeningFailed;
    }
    
    return d->loadDeviceOptions();
}

bool KSaneCore::closeDevice()
{
    if (!d->m_saneHandle) {
        return false;
    }
    stopScan();
    
    disconnect(d->m_scanThread);
    if (d->m_scanThread->isRunning()) {
        connect(d->m_scanThread, &QThread::finished, d->m_scanThread, &QThread::deleteLater);
    }
    if (d->m_scanThread->isFinished()) {
        d->m_scanThread->deleteLater();
    }
    d->m_scanThread = nullptr;
    
    d->m_auth->clearDeviceAuth(d->m_devName);
    sane_close(d->m_saneHandle);
    d->m_saneHandle = nullptr;
    d->clearDeviceOptions();

    return true;
}

void KSaneCore::startScan()
{
    if (!d->m_saneHandle) {
        return;
    }
    d->m_cancelMultiPageScan = false;
    // execute a pending value reload
    while (d->m_readValuesTimer.isActive()) {
        d->m_readValuesTimer.stop();
        d->reloadValues();
    }
    d->m_optionPollTimer.stop();
    d->m_scanThread->start();
}

void KSaneCore::stopScan()
{
    if (!d->m_saneHandle) {
        return;
    }
    
    d->m_cancelMultiPageScan = true;
    if (d->m_scanThread->isRunning()) {
        d->m_scanThread->cancelScan();
    }
}

QImage *KSaneCore::scanImage() const
{
    if (d->m_saneHandle != nullptr) {
        return d->m_scanThread->scanImage();
    }
    return nullptr;
}

void KSaneCore::lockScanImage()
{
    if (d->m_saneHandle != nullptr) {
        d->m_scanThread->lockScanImage();
    }
}

void KSaneCore::unlockScanImage()
{
    if (d->m_saneHandle != nullptr) {
        d->m_scanThread->unlockScanImage();
    }
}

QList<KSaneOption *> KSaneCore::getOptionsList()
{
    return d->m_externalOptionsList;
}

KSaneOption *KSaneCore::getOption(KSaneCore::KSaneOptionName optionEnum) 
{
    auto it = d->m_optionsLocation.find(optionEnum);
    if (it != d->m_optionsLocation.end()) {
        return d->m_externalOptionsList.at(it.value());
    }
    return nullptr;
}

KSaneOption *KSaneCore::getOption(QString optionName) 
{
    for (const auto &option : qAsConst(d->m_externalOptionsList)) {
        if (option->name() == optionName) {
            return option;
        }
    }
    return nullptr;
}

QMap <QString, QString> KSaneCore::getOptionsMap()
{
    KSaneBaseOption *option;
    QMap <QString, QString> options;
    QString tmp;

    for (int i = 0; i < d->m_optionsList.size(); i++) {
        option = d->m_optionsList.at(i);
        tmp = option->valueAsString();
        if (!tmp.isEmpty()) {
            options[option->name()] = tmp;
        }
    }
    return options;
}

int KSaneCore::setOptionsMap(const QMap <QString, QString> &opts)
{
    if (d->m_scanThread->isRunning()) {
        return -1;
    }

    QMap <QString, QString> optionMapCopy = opts;

    QString tmp;
    int i;
    int ret = 0;

    KSaneOption *sourceOption = getOption(SourceOption);
    KSaneOption *modeOption = getOption(ScanModeOption);
    
    // Priorize source option
    if (sourceOption != nullptr && optionMapCopy.contains(sourceOption->name())) {
        if (sourceOption->setValue(optionMapCopy[sourceOption->name()]) ) {
            ret++;
        }
        optionMapCopy.remove(sourceOption->name());
    }

    // Priorize mode option
    if (modeOption != nullptr && optionMapCopy.contains(modeOption->name())) {
        if (modeOption->setValue(optionMapCopy[modeOption->name()])) {
            ret++;
        }
        optionMapCopy.remove(modeOption->name());
    }

    // Update remaining options
    for (i = 0; i < d->m_optionsList.size(); i++) {
        const auto it = optionMapCopy.find(d->m_optionsList.at(i)->name());
        if (it != optionMapCopy.end() && d->m_optionsList.at(i)->setValue(it.value())) {
            ret++;
        }
    }
    return ret;
}

}  // NameSpace KSaneIface
