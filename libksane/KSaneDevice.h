/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2011-12-20
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2011 by Kare Sars <kare.sars@iki.fi>
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

#ifndef KSANEDEVICE_H
#define KSANEDEVICE_H

#include <QObject>
#include <QImage>
#include "libksane_export.h"

class KSaneDevicePrivate;

/**
 * This class provides interface to a scanner and it's options.
 * \author Kare Sars <kare.sars@iki.fi>
 */
class LIBKSANE_EXPORT KSaneDevice : public QObject
{
    Q_OBJECT
    friend class KSaneDevicePrivate;

public:
    /** This enumeration describes the type of the returned data.
     * The number of formats might grow, so it is wise to be prepared fro more.*/
    enum ImageFormat {
        FormatBlackWhite,   /**< One bit per pixel 1 = black 0 = white */
        FormatGrayScale8,   /**< Grayscale with one byte per pixel 0 = black 255 = white */
        FormatGrayScale16,  /**< Grayscale withtTwo bytes per pixel.
                             * The byte order is the one provided by libsane. */
        FormatRGB_8_C,      /**< Every pixel consists of three colors in the order Read,
                             * Grean and Blue, with one byte per color (no alpha channel). */
        FormatRGB_16_C,     /**< Every pixel consists of three colors in the order Read,
                             * Grean and Blue, with two bytes per color(no alpha channel).
                             * The byte order is the one provided by libsane. */
        FormatBMP,          /**< The image data  is returned as a BMP. */
        FormatNone = 0xFFFF /**< This enumeration value should never be returned to the user */
    };

    /** \note There might come more enumerations in the future. */
    enum ScanStatus {
        NoError,            /**< The scanning was finished successfully.*/
        ErrorCannotSegment, /**< If this error status is returned libksane can not segment the
                             * returned data. Scanning without segmentation should work.
                             * \note segmentation is not implemented yet.*/
        ErrorGeneral,        /**< The error string should contain an error message. */
        Information          /**< There is some information to the user. */
    };

    struct DeviceInfo {
        QString vendor;   /**< device vendor string (sometimes called "make")*/
        QString model;    /**< device model name */
        QString type;     /**< device type (e.g., "flatbed scanner") */
        QString name;     /**< unique device name used to identify the scanner */
    };

    explicit KSaneDevice(QObject* parent=0);
    ~KSaneDevice();

    /**
     * Initiate the fetching of the list of connected scanners. The available
     * devices are returned with the availableDevices() signal.
     * \param rescanDeviceList force a rescan of the devices even if a device list
     * already is in memory.
     * \note All sane backends do not support rescanning for devices, which means
     * that the list might not be updated even if a new scanner is connected. */
    void requestDeviceList(bool rescanDeviceList);

    /** This method opens the specified scanner device
     * \param name is the sane device name for the scanner to open.
     * \return 'true' on success and 'false' if the specified scanner can not be opened. */
     bool openDevice(const QString &name);

    /**
     * This is a convenience method that can be used to create a QImage from the image data
     * returned by the imageReady(...) signal. 
     * \note: If the image data has 16 bits/color the * data is truncated to 8 bits/color
     * since QImage does not support 16 bits/color.
     *
     * \param data is the byte data containing the image.
     * \param width is the width of the image in pixels.
     * \param height is the height of the image in pixels.
     * \param bytes_per_line is the number of bytes used per line. This might include padding
     * and is probably only relevant for 'FormatBlackWhite'.
     * \param format is the KSane image format of the data.
     * \return This function returns the provided image data as a QImage. */
    QImage toQImage(const QByteArray &data,
                    int width,
                    int height,
                    int bytes_per_line,
                    ImageFormat format);

    /** This method returns the device information of the open device. If no
     * device is open it will return an empty struct.
     * \return the device information of the open device */
    const DeviceInfo deviceInfo() const;

    /** This method returns the scan area's width in mm
    * \return Width of the scannable area in mm */
    float scanAreaWidth();

    /** This method returns the scan area's height in mm
    * \return Height of the scannable area in mm */
    float scanAreaHeight();

    /** This method sets the selection according to the given points
    * \note The points are defined with respect to the scan areas top-left corner in mm
    * \param topLeft Upper left corner of the selection (in mm)
    * \param bottomRight Lower right corner of the selection (in mm)
    * \return true if the selection could be made and false on failure */
    bool setSelection(QPointF topLeft, QPointF bottomRight);

    /** This function is used to set the preferred resolution for scanning the preview.
    * \param dpi is the wanted scan resolution for the preview
    * \note if the set value is not supported, the cloasest one is used
    * \note setting the value 0 means that the default calculated value should be used */
    void setPreviewResolution(float dpi);

public Q_SLOTS:
    /** This method can be used to cancel a scan or prevent an automatic new scan. */
    void scanCancel();

    /** This method can be used to start a scan (if no GUI is needed).
    * \note libksane may return one or more images as a result of one invocation of this slot.
    * If no more images are wanted scanCancel should be called in the slot handling the
    * imageReady signal. */
    void scanFinal();

Q_SIGNALS:

    /**
     * This signal is emitted every time the device list is updated or
     * after initGetDeviceList() is called.
     * param deviceList is a QList of KSaneDevice::DeviceInfo that contain the
     * device name, model, vendor and type of the attached scanners.
     * \note The list is only a snapshot of the current available devices. Devices
     * might be added or removed/opened after the signal is emitted.
     */
    void availableDevices(const QList<KSaneDevice::DeviceInfo> &deviceList);

    /**
     * This Signal is emitted when a final scan is ready.
     * \param data is the byte data containing the image.
     * \param width is the width of the image in pixels.
     * \param height is the height of the image in pixels.
     * \param bytes_per_line is the number of bytes used per line. This might include padding
     * and is probably only relevant for 'FormatBlackWhite'.
     * \param format is the KSane image format of the data.
     * \param xdpi contains the Dots Per Inch (DPI) of the scanned image for the X-axes
     * \param ydpi contains the Dots Per Inch (DPI) of the scanned image for the Y-axes. */
    void imageReady(QByteArray &data, int width, int height,
                    int bytes_per_line, int format, qreal xdpi, qreal ydpi);

    /**
     * This signal is emitted when the user is to be notified about something.
     * \note If no slot is connected to this signal the message will be displayed on stderr.
     * \param type contains a ScanStatus code to identify the type of message (error/info/...).
     * \param msg is the message to the user.
     * otherwise the string is empty. */
    void userMessage(int type, const QString &strStatus);

    /**
     * This Signal is emitted for progress information during a scan.
     * The GUI already has a progress bar, but if the GUI is hidden,
     * this can be used to display a progress bar.
     * \param percent is the percentage of the scan progress (0-100). */
    void scanProgress(int percent);

    /**
     * This Signal is emitted when a hardware button is pressed.
     * \param optionName is the untranslated technical name of the sane-option.
     * \param optionLabel is the translated user visible label of the sane-option.
     * \param pressed indicates if the value is true or false.
     * \note The SANE standard does not specify hardware buttons and their behaviors,
     * so this signal is emitted for sane-options that behave like hardware buttons.
     * That is the sane-options are read-only and type boolean. The naming of hardware
     * buttons also differ from backend to backend.
     */
    void buttonPressed(const QString &optionName, const QString &optionLabel, bool pressed);

private:
    KSaneDevicePrivate * const d;
};

#endif
