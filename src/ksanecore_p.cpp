/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2008 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2007-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanecore_p.h"

#include <QImage>
#include <QList>

#include <ksane_debug.h>

#include "ksaneinternaloption.h"
#include "ksanebaseoption.h"
#include "ksaneactionoption.h"
#include "ksanebooloption.h"
#include "ksanelistoption.h"
#include "ksanestringoption.h"
#include "ksanedoubleoption.h"
#include "ksanegammaoption.h"
#include "ksaneintegeroption.h"
#include "ksaneinvertoption.h"
#include "ksanepagesizeoption.h"

namespace KSaneIface
{
    
static const QHash<QString, KSaneCore::KSaneOptionName> stringEnumTranslation = {
    { QStringLiteral(SANE_NAME_SCAN_SOURCE), KSaneCore::SourceOption },
    { QStringLiteral(SANE_NAME_SCAN_MODE), KSaneCore::ScanModeOption },
    { QStringLiteral(SANE_NAME_BIT_DEPTH), KSaneCore::BitDepthOption },
    { QStringLiteral(SANE_NAME_SCAN_RESOLUTION), KSaneCore::ResolutionOption },
    { QStringLiteral(SANE_NAME_SCAN_TL_X), KSaneCore::TopLeftXOption },
    { QStringLiteral(SANE_NAME_SCAN_TL_Y), KSaneCore::TopLeftYOption },
    { QStringLiteral(SANE_NAME_SCAN_BR_X), KSaneCore::BottomRightXOption },
    { QStringLiteral(SANE_NAME_SCAN_BR_Y), KSaneCore::BottomRightYOption },
    { QStringLiteral("film-type"), KSaneCore::FilmTypeOption },
    { QStringLiteral(SANE_NAME_NEGATIVE), KSaneCore::NegativeOption },
    { InvertColorsOptionName, KSaneCore::InvertColorOption },
    { PageSizeOptionName, KSaneCore::PageSizeOption },
    { QStringLiteral(SANE_NAME_THRESHOLD), KSaneCore::ThresholdOption },
    { QStringLiteral(SANE_NAME_SCAN_X_RESOLUTION), KSaneCore::XResolutionOption },
    { QStringLiteral(SANE_NAME_SCAN_Y_RESOLUTION), KSaneCore::YResolutionOption },
    { QStringLiteral(SANE_NAME_PREVIEW), KSaneCore::PreviewOption },
    { QStringLiteral("wait-for-button"), KSaneCore::WaitForButtonOption },
    { QStringLiteral(SANE_NAME_BRIGHTNESS), KSaneCore::BrightnessOption },
    { QStringLiteral(SANE_NAME_CONTRAST), KSaneCore::ContrastOption },
    { QStringLiteral(SANE_NAME_GAMMA_VECTOR), KSaneCore::GammaOption },
    { QStringLiteral(SANE_NAME_GAMMA_VECTOR_R), KSaneCore::GammaRedOption },
    { QStringLiteral(SANE_NAME_GAMMA_VECTOR_G), KSaneCore::GammaGreenOption },
    { QStringLiteral(SANE_NAME_GAMMA_VECTOR_B), KSaneCore::GammaBlueOption },
    { QStringLiteral(SANE_NAME_BLACK_LEVEL), KSaneCore::BlackLevelOption },
    { QStringLiteral(SANE_NAME_WHITE_LEVEL), KSaneCore::WhiteLevelOption }, };
    
KSaneCorePrivate::KSaneCorePrivate(KSaneCore *parent):
    q(parent)
{
    clearDeviceOptions();

    m_findDevThread = FindSaneDevicesThread::getInstance();
    connect(m_findDevThread, &FindSaneDevicesThread::finished, this, &KSaneCorePrivate::devicesListUpdated);
    connect(m_findDevThread, &FindSaneDevicesThread::finished, this, &KSaneCorePrivate::signalDevicesListUpdate);

    m_auth = KSaneAuth::getInstance();
    m_optionPollTimer.setInterval(100);
    connect(&m_optionPollTimer, &QTimer::timeout, this, &KSaneCorePrivate::pollPollOptions);
}

KSaneCore::KSaneOpenStatus KSaneCorePrivate::loadDeviceOptions() 
{
    const SANE_Option_Descriptor  *optDesc;
    SANE_Status                    status;
    SANE_Word                      numSaneOptions;
    SANE_Int                       res;
    // update the device list if needed to get the vendor and model info
    if (m_findDevThread->devicesList().size() == 0) {
        m_findDevThread->start();
    } else {
        // use the "old" existing list
        devicesListUpdated();
        // if m_vendor is not updated it means that the list needs to be updated.
        if (m_vendor.isEmpty()) {
            m_findDevThread->start();
        }
    }

    // Read the options (start with option 0 the number of parameters)
    optDesc = sane_get_option_descriptor(m_saneHandle, 0);
    if (optDesc == nullptr) {
        m_auth->clearDeviceAuth(m_devName);
        m_devName.clear();
        return KSaneCore::OpeningFailed;
    }
    QVarLengthArray<char> data(optDesc->size);
    status = sane_control_option(m_saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        m_auth->clearDeviceAuth(m_devName);
        m_devName.clear();
        return KSaneCore::OpeningFailed;
    }
    numSaneOptions = *reinterpret_cast<SANE_Word *>(data.data());

    // read the rest of the options
    // read the rest of the options
    KSaneBaseOption *option = nullptr;
    KSaneBaseOption *optionTopLeftX = nullptr;
    KSaneBaseOption *optionTopLeftY = nullptr;
    KSaneBaseOption *optionBottomRightX = nullptr;
    KSaneBaseOption *optionBottomRightY = nullptr;
    KSaneBaseOption *optionResolution = nullptr;
    m_optionsList.reserve(numSaneOptions);
    m_externalOptionsList.reserve(numSaneOptions);
    for (int i = 1; i < numSaneOptions; ++i) {
        switch (KSaneBaseOption::optionType(sane_get_option_descriptor(m_saneHandle, i))) {
        case KSaneOption::TypeDetectFail:
            option = new KSaneBaseOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeBool:
            option = new KSaneBoolOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeInteger:
            option = new KSaneIntegerOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeDouble:
            option = new KSaneDoubleOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeValueList:
            option = new KSaneListOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeString:
            option = new KSaneStringOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeGamma:
            option = new KSaneGammaOption(m_saneHandle, i);
            break;
        case KSaneOption::TypeAction:
            option = new KSaneActionOption(m_saneHandle, i);
            break;
        }


        if (option->name() == QStringLiteral(SANE_NAME_SCAN_TL_X)) {
            optionTopLeftX = option;
        }
        if (option->name() == QStringLiteral(SANE_NAME_SCAN_TL_Y)) {
            optionTopLeftY = option;
        }
        if (option->name() == QStringLiteral(SANE_NAME_SCAN_BR_X)) {
            optionBottomRightX = option;
        }
        if (option->name() == QStringLiteral(SANE_NAME_SCAN_BR_Y)) {
            optionBottomRightY = option;
        }
        if (option->name() == QStringLiteral(SANE_NAME_SCAN_RESOLUTION)) {
            optionResolution = option;
        }
        if (option->name() == QStringLiteral(SANE_NAME_SCAN_SOURCE)) {
            // some scanners only have ADF and never update the source name
            determineMultiPageScanning(option->value());
            connect(option, &KSaneBaseOption::valueChanged, this, &KSaneCorePrivate::determineMultiPageScanning);
        }
        if (option->name() == QStringLiteral("wait-for-button")) {
            connect(option, &KSaneBaseOption::valueChanged, this, &KSaneCorePrivate::setWaitForExternalButton);
        }
        
        m_optionsList.append(option);
        m_externalOptionsList.append(new KSaneInternalOption(option));
        connect(option, &KSaneBaseOption::optionsNeedReload, this, &KSaneCorePrivate::reloadOptions);
        connect(option, &KSaneBaseOption::valuesNeedReload, this, &KSaneCorePrivate::scheduleValuesReload);

        if (option->needsPolling()) {
            m_optionsPollList.append(option);
            if (option->type() == KSaneOption::TypeBool) {
                connect( option, &KSaneBaseOption::valueChanged,
                    [=]( const QVariant &newValue ) { Q_EMIT q->buttonPressed(option->name(), option->title(), newValue.toBool()); } );
            }
        }
        const auto it = stringEnumTranslation.find(option->name());
        if (it != stringEnumTranslation.constEnd()) {
            m_optionsLocation.insert(it.value(), i - 1);
        }
    }
    
    // add extra option for inverting image colors
    KSaneBaseOption *invertOption = new KSaneInvertOption();
    m_optionsList.append(invertOption);
    m_externalOptionsList.append(new KSaneInternalOption(invertOption));
    m_optionsLocation.insert(KSaneCore::InvertColorOption, m_optionsList.size() - 1);  
    // add extra option for selecting specific page sizes
    KSaneBaseOption *pageSizeOption = new KSanePageSizeOption(optionTopLeftX, optionTopLeftY,
                            optionBottomRightX, optionBottomRightY, optionResolution);
    m_optionsList.append(pageSizeOption);
    m_externalOptionsList.append(new KSaneInternalOption(pageSizeOption));
    m_optionsLocation.insert(KSaneCore::PageSizeOption, m_optionsList.size() - 1);  
  
    // start polling the poll options
    if (m_optionsPollList.size() > 0) {
        m_optionPollTimer.start();
    }

    // Create the scan thread
    m_scanThread = new KSaneScanThread(m_saneHandle);
    connect(m_scanThread, &KSaneScanThread::finished, this, &KSaneCorePrivate::imageScanFinished);
    connect(invertOption, &KSaneInvertOption::valueChanged, m_scanThread, &KSaneScanThread::setImageInverted);
    connect(m_scanThread, &KSaneScanThread::scanProgressUpdated, q, &KSaneCore::scanProgress);

    // try to set to default values
    setDefaultValues();
    return KSaneCore::OpeningSucceeded;
}

void KSaneCorePrivate::clearDeviceOptions()
{
    // delete all the options in the list.
    while (!m_optionsList.isEmpty()) {
        delete m_optionsList.takeFirst();
        delete m_externalOptionsList.takeFirst();
    }
    
    m_optionsLocation.clear();
    m_optionsPollList.clear();
    m_optionPollTimer.stop();

    m_devName.clear();
    m_model.clear();
    m_vendor.clear();
    Q_EMIT q->openedDeviceInfoUpdated(m_devName, m_vendor, m_model);
}

void KSaneCorePrivate::devicesListUpdated()
{
    if (m_vendor.isEmpty()) {
        const QList<KSaneCore::DeviceInfo> deviceList = m_findDevThread->devicesList();
        for (const auto &device : deviceList) {
            if (device.name == m_devName) {
                m_vendor    = device.vendor;
                m_model     = device.model;
                Q_EMIT q->openedDeviceInfoUpdated(m_devName, m_vendor, m_model);
                break;
            }
        }
    }
}

void KSaneCorePrivate::signalDevicesListUpdate()
{
    Q_EMIT q->availableDevices(m_findDevThread->devicesList());
}

void KSaneCorePrivate::setDefaultValues()
{
    KSaneOption *option;

    // Try to get Color mode by default
    if ((option = q->getOption(KSaneCore::ScanModeOption)) != nullptr) {
        option->setValue(i18n(SANE_VALUE_SCAN_MODE_COLOR));
    }

    // Try to set 8 bit color
    if ((option = q->getOption(KSaneCore::BitDepthOption)) != nullptr) {
        option->setValue(8);
    }

    // Try to set Scan resolution to 300 DPI
    if ((option = q->getOption(KSaneCore::ResolutionOption)) != nullptr) {
        option->setValue(300);
    }
}

void KSaneCorePrivate::scheduleValuesReload()
{
    m_readValuesTimer.start(5);
}

void KSaneCorePrivate::reloadOptions()
{
    for (const auto option : qAsConst(m_optionsList)) {
        option->readOption();
        // Also read the values
        option->readValue();
    }
}

void KSaneCorePrivate::reloadValues()
{
    for (const auto option : qAsConst(m_optionsList)) {
        option->readValue();
    }
}

void KSaneCorePrivate::pollPollOptions()
{
    for (int i = 1; i < m_optionsPollList.size(); ++i) {
        m_optionsPollList.at(i)->readValue();
    }
}

void KSaneCorePrivate::imageScanFinished()
{
    if (m_scanThread->frameStatus() == KSaneScanThread::ReadReady) {
        Q_EMIT q->scannedImageReady(*m_scanThread->scanImage());
        // now check if we should have automatic ADF batch scanning
        if (m_executeMultiPageScanning && !m_cancelMultiPageScan) {
            // in batch mode only one area can be scanned per page
            m_scanThread->start();
            return;
        }

        // Check if we have a "wait for button" batch scanning
        if (m_waitForExternalButton) {
            qCDebug(KSANE_LOG) << "waiting for external button press to start next scan";
            m_scanThread->start();
            return;
        }
        scanIsFinished(KSaneCore::NoError, QString());
    } else {
        switch (m_scanThread->saneStatus()) {
        case SANE_STATUS_GOOD:
        case SANE_STATUS_CANCELLED:
        case SANE_STATUS_EOF:
            scanIsFinished(KSaneCore::NoError, sane_i18n(sane_strstatus(m_scanThread->saneStatus())));
            break;
        case SANE_STATUS_NO_DOCS:
            Q_EMIT q->userMessage(KSaneCore::Information, sane_i18n(sane_strstatus(m_scanThread->saneStatus())));
            scanIsFinished(KSaneCore::Information, sane_i18n(sane_strstatus(m_scanThread->saneStatus())));
            break;
        case SANE_STATUS_UNSUPPORTED:
        case SANE_STATUS_IO_ERROR:
        case SANE_STATUS_NO_MEM:
        case SANE_STATUS_INVAL:
        case SANE_STATUS_JAMMED:
        case SANE_STATUS_COVER_OPEN:
        case SANE_STATUS_DEVICE_BUSY:
        case SANE_STATUS_ACCESS_DENIED:
            Q_EMIT q->userMessage(KSaneCore::ErrorGeneral, sane_i18n(sane_strstatus(m_scanThread->saneStatus())));
            scanIsFinished(KSaneCore::ErrorGeneral, sane_i18n(sane_strstatus(m_scanThread->saneStatus())));
            break;
        }
    }
}

void KSaneCorePrivate::scanIsFinished(KSaneCore::KSaneScanStatus status, const QString &message) 
{
    sane_cancel(m_saneHandle);
    if (m_optionsPollList.size() > 0) {
        m_optionPollTimer.start();
    }

    Q_EMIT q->scanFinished(status, message);
}

void KSaneCorePrivate::determineMultiPageScanning(const QVariant &value)
{
    const QString sourceString = value.toString();

    m_executeMultiPageScanning = sourceString.contains(QStringLiteral("Automatic Document Feeder")) ||
    sourceString.contains(sane_i18n("Automatic Document Feeder")) || 
    sourceString.contains(QStringLiteral("ADF")) ||
    sourceString.contains(QStringLiteral("Duplex"));
}

void KSaneCorePrivate::setWaitForExternalButton(const QVariant &value)
{
    m_waitForExternalButton = value.toBool();
}

}  // NameSpace KSaneIface
