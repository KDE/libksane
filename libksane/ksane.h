/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SANE_WIDGET_H
#define SANE_WIDGET_H

// Qt includes.

#include <QWidget>

// Local includes.

#include "libksane_export.h"

/** This namespace collects all methods and classes in LibKSane. */
namespace KSaneIface
{

class KSaneWidgetPriv;

/**
 * This class provides the widget containing the scan options and the preview.
 * @author Kare Sars <kare.sars@iki.fi>
 */
class LIBKSANE_EXPORT KSaneWidget : public QWidget
{
    Q_OBJECT

public:
    /** This enumeration describes the type of the returned data.
     * The number of formats might grow, so it is wise to be prepard fro more.*/
    typedef enum
    {
        FormatBlackWhite,   /**< One bit per pixel 1 = black 0 = white */
        FormatGrayScale8,   /**< Grayscale with one byte per pixel 0 = black 255 = white */
        FormatGrayScale16,  /**< Grayscale withtTwo bytes per pixel.
                             * The byte order is the one provided by libsane. */
        FormatRGB_8_C,      /**< Every pixel consists of three colors in the order Read,
                             * Grean and Blue, with one byte per color (no alpha channel). */
        FormatRGB_16_C,     /**< Every pixel consists of three colors in the order Read,
                             * Grean and Blue, with two bytes per color(no alpha channel).
                             * The byte order is the one provided by libsane. */
        FormatNone = 0xFFFF /**< This enumeration value should never be returned to the user */
    } ImageFormat;

    /** This constructor initializes the private class variables, but the widget is left empty.
     * The options and the preview are added with the call to openDevice(). */
    KSaneWidget(QWidget* parent=0);

    /** Standard destructor */
    ~KSaneWidget();

    /** This helper method displays a dialog for selecting a scanner. The libsane
     * device name of the selected scanner device is returned. */
    QString selectDevice(QWidget* parent=0);

    /** This method opens the specified scanner device and adds the scan options and the
     * preview window to the KSane QWidget.
     * @param device_name is the libsane device name for the scanner to open.
     * @return 'true' if all goes well and 'false' if the specified scanner can not be opened.
     */
    bool    openDevice(const QString &device_name);

    /**
     * This is a convinience method that can be used to create a QImage from the image data
     * returned by the imageReady(...) signal.
     *
     * @param data is the byte data containing the image.
     * @param width is the width of the image in pixels.
     * @param height is the height of the image in pixels.
     * @param bytes_per_line is the number of bytes used per line. This might include padding
     * and is probably only relevant for 'FormatBlackWhite'.
     * @param format is the KSane image format of the data.
     * @param img is the QImage reference to the image to fill.
     */
    bool    makeQImage(const QByteArray &data,
                       int width,
                       int height,
                       int bytes_per_line,
                       ImageFormat format,
                       QImage &img);

    /** This method returns the vendor name of the scanner (Same as make). */
    QString vendor() const;
    /** This method returns the make name of the scanner. */
    QString make() const;
    /** This methos returns the model of the scanner. */
    QString model() const;


public Q_SLOTS:
    /** This method can be used to cancel a scan. */
    void scanCancel();

Q_SIGNALS:
    /**
     * This Signal is emitted when a final scan is ready.
     * @param data is the byte data containing the image.
     * @param width is the width of the image in pixels.
     * @param height is the height of the image in pixels.
     * @param bytes_per_line is the number of bytes used per line. This might include padding
     * and is probably only relevant for 'FormatBlackWhite'.
     * @param format is the KSane image format of the data.
     */
    void imageReady(QByteArray &data, int width, int height,
                    int bytes_per_line, int format);

private Q_SLOTS:

    void scheduleValReload();
    void optReload();
    void valReload();
    void handleSelection(float tl_x, float tl_y, float br_x, float br_y);
    void scanPreview();
    void scanFinal();
    void setTLX(float x);
    void setTLY(float y);
    void setBRX(float x);
    void setBRY(float y);

private:

    void createOptInterface();
    void updatePreviewSize();
    void processData();
    void copyToScanData(int read_bytes);
    void copyToPreview(int read_bytes);
    void setDefaultValues();
    void setBusy(bool busy);

private:

    KSaneWidgetPriv* d;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
