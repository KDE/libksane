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

#define MAX_NUM_OPTIONS 100
#define IMG_DATA_R_SIZE 1000
#define PROGRESS_MAX 100
#define PROGRESS_MIN 0

// Qt includes.

#include <QWidget>
#include <QString>

// Local includes.

#include "libksane_export.h"

namespace KSaneIface
{

class SaneOption;
class SaneWidgetPriv;

typedef enum
{
    READ_NOT_READING,
    READ_ON_GOING,
    READ_ERROR,
    READ_CANCEL,
    READ_FINISHED
} ReadStatus;

class LIBKSANE_EXPORT SaneWidget : public QWidget
{
    Q_OBJECT

public:

    SaneWidget(QWidget* parent=0);
    ~SaneWidget();

    QString selectDevice(QWidget* parent=0);
    bool    openDevice(const QString &device_name);
    QImage *getFinalImage();

    QString vendor() const;
    QString make() const;
    QString model() const;

    bool setIconColorMode(const QIcon &icon);
    bool setIconGrayMode(const QIcon &icon);
    bool setIconBWMode(const QIcon &icon);

public Q_SLOTS:

    void scanCancel();

Q_SIGNALS:

    void imageReady();
    void scanFaild();

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

    SaneOption *getOption(const QString &name);
    void createOptInterface();
    void updatePreviewSize();
    void processData();
    void setDefaultValues();
    void setBusy(bool busy);

private:

    SaneWidgetPriv* d;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
