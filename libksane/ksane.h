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
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SANE_WIDGET_H
#define SANE_WIDGET_H

// Qt includes.

#include <QWidget>

// Local includes.

#include "libksane_export.h"

namespace KSaneIface
{

class KSaneWidgetPriv;

class LIBKSANE_EXPORT KSaneWidget : public QWidget
{
    Q_OBJECT

public:
    typedef enum
    {
        FormatBlackWhite,
        FormatGrayScale8,
        FormatGrayScale16,
        FormatRGB_8_C,
        FormatRGB_16_C,
        FormatNone
    } ImageFormat;


    KSaneWidget(QWidget* parent=0);
    ~KSaneWidget();

    QString selectDevice(QWidget* parent=0);
    bool    openDevice(const QString &device_name);
    bool    makeQImage(const QByteArray &data,
                       int width,
                       int height,
                       int bytes_per_line,
                       ImageFormat format,
                       QImage &img);

    QString vendor() const;
    QString make() const;
    QString model() const;


public Q_SLOTS:

    void scanCancel();

Q_SIGNALS:

    void imageReady(QByteArray &data, int width, int height,
                    int bytesperline, int format);

private Q_SLOTS:

    void opt_level_change(int level);
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
