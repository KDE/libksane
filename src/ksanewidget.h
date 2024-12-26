/*
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2007 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef KSANE_H
#define KSANE_H

#include "ksane_export.h"

#include <QWidget>

namespace KSaneIface
{

class KSaneWidgetPrivate;

/**
 * This class provides the widget containing the scan options and the preview.
 * @author Kare Sars <kare.sars@iki.fi>
 */
class KSANE_EXPORT KSaneWidget : public QWidget
{
    Q_OBJECT
    friend class KSaneWidgetPrivate;

public:

    /** @note There might come more enumerations in the future. */
    typedef enum {
        NoError,            /**< The scanning was finished successfully.*/
        ErrorCannotSegment, /**< If this error status is returned libksane can not segment the
                             * returned data. Scanning without segmentation should work.
                             * @note segmentation is not implemented yet.*/
        ErrorGeneral,        /**< The error string should contain an error message. */
        Information          /**< There is some information to the user. */
    } ScanStatus;

    struct DeviceInfo {
        QString name;     /* unique device name */
        QString vendor;   /* device vendor string */
        QString model;    /* device model name */
        QString type;     /* device type (e.g., "flatbed scanner") */
    };

    /** This constructor initializes the private class variables, but the widget is left empty.
     * The options and the preview are added with the call to openDevice(). */
    KSaneWidget(QWidget *parent = nullptr);

    /** Standard destructor */
    ~KSaneWidget() override;

    /** This helper method displays a dialog for selecting a scanner. The libsane
     * device name of the selected scanner device is returned. */
    QString selectDevice(QWidget *parent = nullptr);

    /** This method opens the specified scanner device and adds the scan options to the
     * KSane widget.
     * @param device_name is the libsane device name for the scanner to open.
     * @return 'true' if all goes well and 'false' if the specified scanner can not be opened. */
    bool openDevice(const QString &device_name);

    /** This method closes the currently open scanner device.
    * @return 'true' if all goes well and 'false' if no device is open. */
    bool closeDevice();

    /** This method returns the internal device name of the currently opened scanner. */
    QString deviceName() const;

    /** This method returns the vendor name of the currently opened scanner. */
    QString deviceVendor() const;

    /** This method returns the model of the currently opened scanner. */
    QString deviceModel() const;

    /** This method returns the scan area's width in mm
    * @return Width of the scannable area in mm */
    float scanAreaWidth();

    /** This method returns the scan area's height in mm
    * @return Height of the scannable area in mm */
    float scanAreaHeight();

    /** This method sets the selection according to the given points
     * @note The points are defined with respect to the scan areas top-left corner in mm
     * @param topLeft Upper left corner of the selection (in mm)
     * @param bottomRight Lower right corner of the selection (in mm) */
    void setSelection(QPointF topLeft, QPointF bottomRight);

    /** This function is used to set the preferred resolution for scanning the preview.
     * @param dpi is the wanted scan resolution for the preview
     * @note if the set value is not supported, the cloasest one is used
     * @note setting the value 0 means that the default calculated value should be used */
    void setPreviewResolution(float dpi);

    /** This method reads the available parameters and their values and
     * returns them in a QMap (Name, value)
     * @param opts is a QMap with the parameter names and values. */
    void getOptionValues(QMap <QString, QString> &options);

    /** This method can be used to write many parameter values at once.
     * @param opts is a QMap with the parameter names and values.
     * @return This function returns the number of successful writes
     * or -1 if scanning is in progress. */
    int setOptionValues(const QMap <QString, QString> &options);

    /** This function reads one parameter value into a string.
     * @param optname is the name of the parameter to read.
     * @param value is the string representation of the value.
     * @return this function returns true if the read was successful. */
    bool getOptionValue(const QString &option, QString &value);

    /** This function writes one parameter value into a string.
     * @param optname is the name of the parameter to write.
     * @param value is the string representation of the value.
     * @return this function returns true if the write was successful and
     * false if it was unsuccessful or scanning is in progress. */
    bool setOptionValue(const QString &option, const QString &value);

    /** This function can be used to enable/disable automatic selections on previews.
    * The default state is enabled.
    * @param enable specifies if the auto selection should be turned on or off. */
    void enableAutoSelect(bool enable);

public Q_SLOTS:
    /** This method can be used to cancel a scan or prevent an automatic new scan. */
    void cancelScan();

    /** This method can be used to start a scan (if no GUI is needed).
    * @note libksane may return one or more images as a result of one invocation of this slot.
    * If no more images are wanted cancelScan should be called in the slot handling the
    * imageReady signal. */
    void startScan();

    /** This method can be used to start a preview scan. */
    void startPreviewScan();

Q_SIGNALS:
    /**
     * This signal is emitted when a final scan is ready.
     * @param scannedImage is the QImage containing the scanned image data. */
    void scannedImageReady(const QImage &scannedImage);

    /**
     * This signal is emitted when the scanning has ended.
     * @param status contains a ScanStatus status code.
     * @param strStatus If an error has occurred this string will contain an error message.
     * otherwise the string is empty. */
    void scanDone(int status, const QString &strStatus);

    /**
     * This signal is emitted when the user is to be notified about something.
     * @note If no slot is connected to this signal the message will be displayed in a KMessageBox.
     * @param type contains a ScanStatus code to identify the type of message (error/info/...).
     * @param strStatus If an error has occurred this string will contain an error message.
     * otherwise the string is empty. */
    void userMessage(int type, const QString &strStatus);

    /**
     * This Signal is emitted for progress information during a scan.
     * The GUI already has a progress bar, but if the GUI is hidden,
     * this can be used to display a progress bar.
     * @param percent is the percentage of the scan progress (0-100). */
    void scanProgress(int percent);

    /**
     * This signal is emitted every time the device list is updated or
     * after initGetDeviceList() is called.
     * @param deviceList is a QList of KSaneWidget::DeviceInfo that contain the
     * device name, model, vendor and type of the attached scanners.
     * @note The list is only a snapshot of the current available devices. Devices
     * might be added or removed/opened after the signal is emitted.
     */
    void availableDevices(const QList<KSaneIface::KSaneWidget::DeviceInfo> &deviceList);

    /**
     * This Signal is emitted when a hardware button is pressed.
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
     * This signal is not emitted anymore.
     */
    void openedDeviceInfoUpdated(const QString &deviceName, const QString &deivceVendor, const QString &deviceModel);

private:
    KSaneWidgetPrivate *const d;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
