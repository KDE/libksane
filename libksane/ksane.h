/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2012 by Kare Sars <kare.sars@iki.fi>
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

#ifndef KSaneWidget_h
#define KSaneWidget_h

// Qt includes
#include <QtGui/QWidget>

// Local includes
#include "libksane_export.h"

class KSaneWidgetPrivate;

/**
 * This class provides the widget containing the scan options and the preview.
 * @author Kare Sars <kare.sars@iki.fi>
 */
class LIBKSANE_EXPORT KSaneWidget : public QWidget
{
    Q_OBJECT
    friend class KSaneWidgetPrivate;

public:
    /** This constructor initializes the (private) class variables, but the widget is left empty.
     * The options and the preview are added with the call to openDevice(). */
    KSaneWidget(QWidget* parent=0);

    /** Standard destructor */
    ~KSaneWidget();

    /** This helper method displays a dialog for selecting a scanner. The libsane
     * device name of the selected scanner device is returned. */
    QString selectDevice(QWidget* parent=0);

    /** This method opens the specified scanner device and adds the scan options to the
     * KSane widget.
     * @param device_name is the libsane device name for the scanner to open.
     * @return 'true' if all goes well and 'false' if the specified scanner can not be opened. */
    bool openDevice(const QString &device_name);

    /** This method closes the currently open scanner device.
    * @return 'true' if all goes well and 'false' if no device is open. */
    bool closeDevice();

    /** This method reads the available parameters and their values and
     * returns them in a QMap (Name, value)
     * @param opts is a QMap with the parameter names and values. */
    void getOptVals(QMap <QString, QString> &opts);

    /** This method can be used to write many parameter values at once.
     * @param opts is a QMap with the parameter names and values.
     * @return This function returns the number of successful writes. */
    int setOptVals(const QMap <QString, QString> &opts);

    /** This function reads one parameter value into a string.
     * @param optname is the name of the parameter to read.
     * @param value is the string representation of the value.
     * @return this function returns true if the read was successful. */
    bool getOptVal(const QString &optname, QString &value);

    /** This function writes one parameter value into a string.
     * @param optname is the name of the parameter to write.
     * @param value is the string representation of the value.
     * @return this function returns true if the write was successful. */
    bool setOptVal(const QString &optname, const QString &value);

    /** This function sets the label on the final scan button
    * @param scanLabel is the new label for the button. */
    void setScanButtonText(const QString &scanLabel);

    /** This function sets the label on the preview button
    * @param previewLabel is the new label for the button. */
    void setPreviewButtonText(const QString &previewLabel);

    /** This function can be used to enable/disable automatic selections on previews.
    * The default state is enabled.
    * @param enable specifies if the auto selection should be turned on or off. */
    void enableAutoSelect(bool enable);

    /** This function is used to programatically collapse/restore the options.
    * @param collapse defines the state to set. */
    void setOptionsCollapsed(bool collapse);

    /** This function is used hide/show the final scan button.
    * @param hidden defines the state to set. */
    void setScanButtonHidden(bool hidden);


private:

    KSaneWidgetPrivate * const d;
};

#endif
