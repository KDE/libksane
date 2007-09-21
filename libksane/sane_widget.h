/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
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

#include <QScrollArea>
#include <QWidget>
#include <QString>
#include <QToolBar>
#include <QTimer>

// Sane includes.

extern "C" 
{
#include <sane/sane.h>
}

// Local includes.

#include "libksane_export.h"

class QPushButton;

namespace KSaneIface
{

class SaneOption;
class PreviewArea;

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

    QString make() const;
    QString model() const;

    bool setIconColorMode(const QIcon &icon);
    bool setIconGrayMode(const QIcon &icon);
    bool setIconBWMode(const QIcon &icon);

public Q_SLOTS:

    void scanCancel();

Q_SIGNALS:

    void scanStart();
    void scanProgress(int);
    void scanDone();
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

private:

    // device info
    SANE_Device const **dev_list;
    SANE_Handle         s_handle;
    QString             devname;
    QString             modelname;
    QString             m_make;
    QString             m_model;

    // Option variables
    QScrollArea        *opt_area;
    bool                options_read;
    QList<SaneOption *> optList;
    SaneOption         *opt_mode;
    SaneOption         *opt_depth;
    SaneOption         *opt_res;
    SaneOption         *opt_res_y;
    SaneOption         *opt_tl_x;
    SaneOption         *opt_tl_y;
    SaneOption         *opt_br_x;
    SaneOption         *opt_br_y;
    QWidget            *color_opts;
    QWidget            *remain_opts;
    SaneOption         *opt_gam_r;
    SaneOption         *opt_gam_g;
    SaneOption         *opt_gam_b;
    QTimer              r_val_tmr;

    QPushButton        *scan_btn;
    QPushButton        *prev_btn;

    QPushButton        *z_in_btn;
    QPushButton        *z_out_btn;
    QPushButton        *z_sel_btn;
    QPushButton        *z_fit_btn;

    // preview variables
    PreviewArea        *preview;
    QImage             *pr_img;
    float               previewWidth;
    float               previewHeight;

    // general scanning
    SANE_Parameters     params;

    //QSocketNotifier *sn;
    int                 progress;
    SANE_Byte           img_data[IMG_DATA_R_SIZE];
    SANE_Byte           px_colors[3];
    unsigned int        px_c_index;
    int                 pixel_x, pixel_y;
    ReadStatus          read_status;
    QImage             *scan_img;
    QImage              the_img;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
