/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2007 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_CORE_H
#define KSANE_CORE_H

#include "ksane_export.h"

#include <memory>

#include <QObject>
#include <QList>
#include <QImage>

/** This namespace collects all methods and classes in LibKSane. */
namespace KSaneIface
{

class KSaneCorePrivate;
class KSaneOption;
    
/**
 * This class provides the core interface for accessing the scan controls and options.
 */
class KSANE_EXPORT KSaneCore : public QObject
{
    Q_OBJECT
    friend class KSaneCorePrivate;

public:
    /** @note There might come more enumerations in the future. */
    enum KSaneScanStatus {
        NoError,            // The scanning was finished successfully
        ErrorGeneral,       // The error string should contain an error message.
        Information         // There is some information to the user.
    };
    
    /** Enum determining whether the scanner opened correctly. */
    enum KSaneOpenStatus {
        OpeningSucceeded, // scanner opened successfully
        OpeningDenied, // access was denied, 
        OpeningFailed, // opening the scanner failed for unknown reasons
    };  
    
    /** 
     * This enumeration is used to obtain a specific option with getOption(KSaneOptionName).
     * Depending on the backend, not all options are available, nor this list is complete.
     * For the remaining options, getOptionsList() must be used. 
     */   
    enum KSaneOptionName {
        SourceOption,
        ScanModeOption,
        BitDepthOption,
        ResolutionOption,
        TopLeftXOption,
        TopLeftYOption,
        BottomRightXOption,
        BottomRightYOption,
        FilmTypeOption,
        NegativeOption,
        InvertColorOption,
        PageSizeOption,
        ThresholdOption,
        XResolutionOption,
        YResolutionOption,
        PreviewOption,
        WaitForButtonOption,
        BrightnessOption,
        ContrastOption,
        GammaOption,
        GammaRedOption,
        GammaGreenOption,
        GammaBlueOption,
        BlackLevelOption,
        WhiteLevelOption,
    };

    struct DeviceInfo {
        QString name;     /* unique device name */
        QString vendor;   /* device vendor string */
        QString model;    /* device model name */
        QString type;     /* device type (e.g., "flatbed scanner") */
    };

    /**
     * This constructor initializes the private class variables, but the widget is left empty.
     * The options and the preview are added with the call to openDevice().
     */
    KSaneCore(QObject *parent = nullptr);

    /** Standard destructor */
    ~KSaneCore();

    /**
     * Get the list of available scanning devices. Connect to availableDevices()
     * which is fired once these devices are known.
     * @return whether the devices list are reloaded or not.
     */
    bool reloadDevicesList();

    /**
     * This method opens the specified scanner device and adds the scan options to the
     * options list.
     * @param deviceName is the libsane device name for the scanner to open.
     * @return the status of the opening action.
     */
    KSaneOpenStatus openDevice(const QString &deviceName);
    
    /**
     * This method opens the specified scanner device with a specified username and password.
     * Adds the scan options to the options list.
     * @param deviceName is the libsane device name for the scanner to open.
     * @param userName the username required to open for the scanner.
     * @param password the password required to open for the scanner.
     * @return the status of the opening action.
     */
    KSaneOpenStatus openRestrictedDevice(const QString &deviceName, QString userName, QString password);

    /**
     * This method closes the currently open scanner device.
     * @return 'true' if all goes well and 'false' if no device is open.
     */
    bool closeDevice();

    /**
     * This method returns the internal device name of the currently opened scanner.
     */
    QString deviceName() const;
    
    /**
     * This method returns the vendor name of the currently opened scanner. 
     */
    QString deviceVendor() const;  
    
    /**
     * This method returns the model of the currently opened scanner.
     */
    QString deviceModel() const;

    /**
     * This function returns all available options when a device is opened.
     * @return list containing pointers to all KSaneOptions provided by the backend.
     * Becomes invalid when closing a device. 
     * The pointers must not be deleted by the client.
     */
    QList<KSaneOption *> getOptionsList();
    
    /**
     * This function returns a specific option. 
     * @param optionEnum the enum specifying the option.
     * @return pointer to the KSaneOption. Returns a nullptr in case the options
     * is not available for the currently opened device.
     */  
    KSaneOption *getOption(KSaneOptionName optionEnum);
    
    /**
     * This function returns a specific option. 
     * @param optionName the internal name of the option defined by SANE.
     * @return pointer to the KSaneOption. Returns a nullptr in case the options
     * is not available for the currently opened device.
     */  
    KSaneOption *getOption(QString optionName); 
    
    /**
     * This method reads the available parameters and their values and
     * returns them in a QMap (Name, value)
     * @param opts is a QMap with the parameter names and values.
     */
    QMap <QString, QString> getOptionsMap();

    /**
     * This method can be used to write many parameter values at once.
     * @param opts is a QMap with the parameter names and values.
     * @return This function returns the number of successful writes
     * or -1 if scanning is in progress.
     */
    int setOptionsMap(const QMap <QString, QString> &opts);
    
    /**
     * Gives direct access to the QImage that is used to store the image
     * data retrieved from the scanner.
     * Useful to display the an in-progress image while scanning.
     * When accessing the direct image pointer during a scan, the image
     * must be locked before accessing the image and unlocked afterwards
     * using the lockScanImage() and unlockScanImage() functions.
     * @return pointer for direct access of the QImage data.
     */
    QImage *scanImage() const;
    
    /**
     * Locks the mutex protecting the QImage pointer of scanImage() from
     * concurrent access during scanning.
     */
    void lockScanImage();
    
    /**
     * Unlocks the mutex protecting the QImage pointer of scanImage() from
     * concurrent access during scanning. The scanning progress will blocked
     * when lockScanImage() is called until unlockScanImage() is called.
     */
    void unlockScanImage();

public Q_SLOTS:
    /**
     * This method can be used to cancel a scan or prevent an automatic new scan.
     */
    void stopScan();

    /**
     * This method can be used to start a scan (if no GUI is needed).
     * @note libksane may return one or more images as a result of one invocation of this slot.
     * If no more images are wanted scanCancel should be called in the slot handling the
     * imageReady signal.
     */
    void startScan();

Q_SIGNALS:
    /**
     * This signal is emitted when a final scan is ready.
     * @param scannedImage is the QImage containing the scanned image data.
     */
    void scannedImageReady(const QImage &scannedImage);

    /**
     * This signal is emitted when the scanning has ended.
     * @param status contains a ScanStatus status code.
     * @param strStatus If an error has occurred this string will contain an error message.
     * otherwise the string is empty.
     */
    void scanFinished(KSaneScanStatus status, const QString &strStatus);

    /**
     * This signal is emitted when the user is to be notified about something.
     * @note If no slot is connected to this signal the message will be displayed in a KMessageBox.
     * @param type contains a ScanStatus code to identify the type of message (error/info/...).
     * @param strStatus If an error has occurred this string will contain an error message.
     * otherwise the string is empty.
     */
    void userMessage(KSaneScanStatus status, const QString &strStatus);

    /**
     * This signal is emitted for progress information during a scan.
     * The GUI already has a progress bar, but if the GUI is hidden,
     * this can be used to display a progress bar.
     * @param percent is the percentage of the scan progress (0-100).
     */
    void scanProgress(int percent);

    /**
     * This signal is emitted every time the device list is updated or
     * after initGetDeviceList() is called.
     * @param deviceList is a QList of KSaneCore::DeviceInfo that contain the
     * device name, model, vendor and type of the attached scanners.
     * @note The list is only a snapshot of the current available devices. Devices
     * might be added or removed/opened after the signal is emitted.
     */
    void availableDevices(const QList<KSaneCore::DeviceInfo> &deviceList);

    /**
     * This signal is emitted when a hardware button is pressed.
     * @param optionName is the untranslated technical name of the sane-option.
     * @param optionLabel is the translated user visible label of the sane-option.
     * @param pressed indicates if the value is true or false.
     * @note The SANE standard does not specify hardware buttons and their behaviors,
     * so this signal is emitted for sane-options that behave like hardware buttons.
     * That is the sane-options are read-only and type boolean. The naming of hardware
     * buttons also differ from backend to backend.
     */
    void buttonPressed(const QString &optionName, const QString &optionLabel, bool pressed);
    
    /**
     * This signal is emitted when the device info of the already opened scanner device
     * is updated and vendor() and model() return the corresponding names.
     * @param deviceName is the technical device name of the currently opened scanner.
     * @param deivceVendor is the vendor of the currently opened scanner.
     * @param deviceModel is the model name of the currently opened scanner.
     */
    void openedDeviceInfoUpdated(const QString &deviceName, const QString &deivceVendor, const QString &deviceModel);
    
private:
    std::unique_ptr<KSaneCorePrivate> d;
};

}  // NameSpace KSaneIface

#endif // KSANE_CORE_H
