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
    SANE_Handle         m_saneHandle;
    QString             m_modelName;
    QString             m_make;
    QString             m_model;

    // Option variables
    QScrollArea        *m_optArea;
    QList<SaneOption*>  m_optList;
    SaneOption         *m_optMode;
    SaneOption         *m_optDepth;
    SaneOption         *m_optRes;
    SaneOption         *m_optResY;
    SaneOption         *m_optTl;
    SaneOption         *m_optTlY;
    SaneOption         *m_optBrX;
    SaneOption         *m_optBrY;
    SaneOption         *m_optGamR;
    SaneOption         *m_optGamG;
    SaneOption         *m_optGamB;
    QWidget            *m_colorOpts;
    QWidget            *m_remainOpts;
    QTimer              m_rValTmr;

    QPushButton        *m_scanBtn;
    QPushButton        *m_prevBtn;

    QPushButton        *m_zInBtn;
    QPushButton        *m_zOutBtn;
    QPushButton        *m_zSelBtn;
    QPushButton        *m_zFitBtn;

    // preview variables
    PreviewArea        *m_preview;
    QImage             *m_prImg;
    float               m_previewWidth;
    float               m_previewHeight;

    // general scanning
    unsigned int        m_pxCIndex;
    int                 m_progress;
    int                 m_pixelX;
    int                 m_pixelY;
    SANE_Parameters     m_params;
    SANE_Byte           m_imgData[IMG_DATA_R_SIZE];
    SANE_Byte           m_pxColors[3];
    ReadStatus          m_readStatus;
    QImage             *m_scanImg;
    QImage              m_theImg;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
