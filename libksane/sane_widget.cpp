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

#define ENABLE_DEBUG

// C++ includes.
#include <iostream>

// Sane includes.
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

// Qt includes.
#include <QEventLoop>
#include <QApplication>
#include <QVarLengthArray>
#include <QComboBox>
#include <QImage>
#include <QTimer>
#include <QScrollArea>
#include <QList>
#include <QProgressBar>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

// Local includes.
#include "sane_option.h"
#include "preview_area.h"
#include "sane_option.h"
#include "labeled_separator.h"
#include "radio_select.h"
#include "labeled_gamma.h"
#include "ksane.h"
#include "ksane.moc"

#define SCALED_PREVIEW_MAX_SIDE 400
#define MAX_NUM_OPTIONS 100
#define IMG_DATA_R_SIZE 10000

namespace KSaneIface
{

typedef enum
{
    READ_ON_GOING,
    READ_ERROR,
    READ_CANCEL,
    READ_FINISHED
} ReadStatus;


class KSaneWidgetPriv
{
public:

    KSaneWidgetPriv()
    {
        optArea       = 0;
        optMode       = 0;
        optDepth      = 0;
        optRes        = 0;
        optResY       = 0;
        optTl         = 0;
        optTlY        = 0;
        optBrX        = 0;
        optBrY        = 0;
        optGamR       = 0;
        optGamG       = 0;
        optGamB       = 0;
        colorOpts     = 0;
        remainOpts    = 0;
        scanBtn       = 0;
        prevBtn       = 0;
        zInBtn        = 0;
        zOutBtn       = 0;
        zSelBtn       = 0;
        zFitBtn       = 0;
        cancelBtn     = 0;
        previewArea   = 0;
        previewImg    = 0;
        previewWidth  = 0;
        previewHeight = 0;
        pixel_x       = 0;
        pixel_y       = 0;
        isPreview     = false;
        frameSize     = 0;
        frameRead     = 0;
        dataSize      = 0;
        progress      = 0;
        readStatus    = READ_ON_GOING;
        progressBar   = 0;
        px_c_index    = 0;
        frame_t_count = 0;
    }

    KSaneWidget::ImageFormat getImgFormat()
    {
        switch(params.format)
        {
            case SANE_FRAME_GRAY:
                switch(params.depth)
                {
                    case 1:
                        return KSaneWidget::FormatBlackWhite;
                    case 8:
                        return KSaneWidget::FormatGrayScale8;
                    case 16:
                        return KSaneWidget::FormatGrayScale16;
                    default:
                        return KSaneWidget::FormatNone;
                }
            case SANE_FRAME_RGB:
            case SANE_FRAME_RED:
            case SANE_FRAME_GREEN:
            case SANE_FRAME_BLUE:
                switch (params.depth)
                {
                    case 8:
                        return KSaneWidget::FormatRGB_8_C;
                    case 16:
                        return KSaneWidget::FormatRGB_16_C;
                    default:
                        return KSaneWidget::FormatNone;
                }
        }
        return KSaneWidget::FormatNone;
    }

    int getBytesPerLines()
    {
        switch (getImgFormat())
        {
            case KSaneWidget::FormatBlackWhite:
            case KSaneWidget::FormatGrayScale8:
            case KSaneWidget::FormatGrayScale16:
                return params.bytes_per_line;

            case KSaneWidget::FormatRGB_8_C:
                return params.pixels_per_line*4;

            case KSaneWidget::FormatRGB_16_C:
                return params.pixels_per_line*8;

            case KSaneWidget::FormatNone:
                return 0;
        }
        return 0;
    }

    SaneOption *getOption(const QString &name)
    {
        int i;
        for (i=0; i<optList.size(); i++) {
            if (optList.at(i)->name() == name) {
                return optList.at(i);
            }
        }
        return 0;
    }


    // device info
    SANE_Handle         saneHandle;
    QString             modelName;
    QString             vendor;
    QString             model;

    // Option variables
    QScrollArea        *optArea;
    QList<SaneOption*>  optList;
    SaneOption         *optMode;
    SaneOption         *optDepth;
    SaneOption         *optRes;
    SaneOption         *optResY;
    SaneOption         *optTl;
    SaneOption         *optTlY;
    SaneOption         *optBrX;
    SaneOption         *optBrY;
    SaneOption         *optGamR;
    SaneOption         *optGamG;
    SaneOption         *optGamB;
    QWidget            *colorOpts;
    QWidget            *remainOpts;
    QTimer              rValTmr;

    QPushButton        *scanBtn;
    QPushButton        *prevBtn;
    QPushButton        *zInBtn;
    QPushButton        *zOutBtn;
    QPushButton        *zSelBtn;
    QPushButton        *zFitBtn;
    QPushButton        *cancelBtn;

    QProgressBar       *progressBar;

    // preview variables
    PreviewArea        *previewArea;
    float               previewWidth;
    float               previewHeight;
    QImage             *previewImg;
    bool                isPreview;
    int                 pixel_x;
    int                 pixel_y;
    int                 px_colors[3];
    int                 px_c_index;

    // final image data
    QByteArray          scanData;

    // general scanning
    ReadStatus          readStatus;
    int                 progress;
    SANE_Parameters     params;
    SANE_Byte           saneReadBuffer[IMG_DATA_R_SIZE];
    int                 frameSize;
    int                 frameRead;
    int                 dataSize;
    int                 frame_t_count;
};

KSaneWidget::KSaneWidget(QWidget* parent)
          : QWidget(parent)
{
    d = new KSaneWidgetPriv;

    SANE_Int    version;
    SANE_Status status;
    status = sane_init(&version, 0);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "libksane: sane_init() failed("
                 << sane_strstatus(status) << ")";
    }
    else {
        kDebug() << "Sane Version = "
                 << SANE_VERSION_MAJOR(version) << "."
                 << SANE_VERSION_MINOR(version) << "."
                 << SANE_VERSION_BUILD(version);
    }
    d->rValTmr.setSingleShot(true);

    connect (&d->rValTmr, SIGNAL(timeout()),
             this, SLOT(valReload()));
}

KSaneWidget::~KSaneWidget()
{
    d->optList.clear();
    sane_exit();
    delete d;
}

QString KSaneWidget::vendor() const
{
    return d->vendor;
}

QString KSaneWidget::make() const
{
    return d->vendor;
}

QString KSaneWidget::model() const
{
    return d->model;
}

QString KSaneWidget::selectDevice(QWidget* parent)
{
    int                 i=0;
    int                 num_scaners;
    QStringList         dev_name_list;
    QString             tmp;
    SANE_Status         status;
    SANE_Device const **dev_list;

    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0) {
        kDebug() << "i="       << i << " "
                 << "name='"   << dev_list[i]->name   << "' "
                 << "vendor='" << dev_list[i]->vendor << "' "
                 << "model='"  << dev_list[i]->model  << "' "
                 << "type='"   << dev_list[i]->type   << "'";
        tmp = QString(dev_list[i]->name);
        tmp += "\n" + QString(dev_list[i]->vendor);
        tmp += " : " + QString(dev_list[i]->model);
        dev_name_list += tmp;
        i++;
    }
    num_scaners = i;

    // add the debug test scanner to the end
#ifdef ENABLE_DEBUG
    tmp = QString("test:0");
    tmp += "\n" + QString("Tester");
    tmp += " : " + QString("Model");
    dev_name_list += tmp;
#endif

    if (dev_name_list.isEmpty()) {
        KMessageBox::sorry(0, i18n("No scanner device has been found."));
        return QString();
    }

    RadioSelect sel;
    sel.setWindowTitle(qApp->applicationName());
    i = sel.getSelectedIndex(parent, QString("Select Scanner"), dev_name_list, 0);
    kDebug() << "i=" << i;

    if (i == num_scaners) {
        return QString("test:0");
    }

    if ((i < 0) || (i >= num_scaners)) {
        return QString();
    }

    return QString(dev_list[i]->name);
}

bool KSaneWidget::openDevice(const QString &device_name)
{
    int                            i=0;
    const SANE_Option_Descriptor  *num_option_d;
    SANE_Status                    status;
    SANE_Word                      num_sane_options;
    SANE_Int                       res;
    SANE_Device const            **dev_list;

    // get the device list to get the vendor and model info
    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0) {
        if (QString(dev_list[i]->name) == device_name) {
            d->modelName = QString(dev_list[i]->vendor) + " " + QString(dev_list[i]->model);
            d->vendor    = QString(dev_list[i]->vendor);
            d->model     = QString(dev_list[i]->model);
            break;
        }
        i++;
    }

    if (dev_list[i] == 0)
    {
#ifdef ENABLE_DEBUG
        d->modelName = i18n("Test Scanner");
        d->vendor    = QString("Test");
        d->model     = QString("Debug");
#else
        kDebug() << "openDevice: '" << qPrintable(device_name) << "' not found";
        return false;
#endif
    }

    // Try to open the device
    if (sane_open(device_name.toLatin1(), &d->saneHandle) != SANE_STATUS_GOOD) {
        kDebug() << "openDevice: sane_open(\"" << qPrintable(device_name)
                 << "\", &handle) failed!";
        return false;
    }
    //printf("openDevice: sane_open(\"%s\", &handle) == SANE_STATUS_GOOD\n", qPrintable(device_name));

    // Read the options (start with option 0 the number of parameters)
    num_option_d = sane_get_option_descriptor(d->saneHandle, 0);
    if (num_option_d == 0) {
        return false;
    }
    QVarLengthArray<char> data(num_option_d->size);
    status = sane_control_option(d->saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return false;
    }
    num_sane_options = *reinterpret_cast<SANE_Word*>(data.data());

    // read the rest of the options
    for (i=1; i<num_sane_options; i++) {
        d->optList.append(new SaneOption(d->saneHandle, i));
    }

    // do the connections of the option parameters
    for (i=1; i<d->optList.size(); i++) {
        connect (d->optList.at(i), SIGNAL(optsNeedReload()),
                 this, SLOT(optReload()));

        connect (d->optList.at(i), SIGNAL(valsNeedReload()),
                 this, SLOT(scheduleValReload()));
    }

    // create the layout
    QHBoxLayout *base_layout = new QHBoxLayout;
    base_layout->setSpacing(2);
    base_layout->setMargin(0);
    setLayout(base_layout);
    QVBoxLayout *opt_lay = new QVBoxLayout;
    opt_lay->setSpacing(2);
    opt_lay->setMargin(0);
    QVBoxLayout *pr_layout = new QVBoxLayout;

    base_layout->addLayout(opt_lay, 0);
    base_layout->addLayout(pr_layout, 100);

    // Create Option Scroll Area
    d->optArea = new QScrollArea(this);
    d->optArea->setWidgetResizable(true);
    d->optArea->setFrameShape(QFrame::NoFrame);
    opt_lay->addWidget(d->optArea, 0);
    opt_lay->setSpacing(2);
    opt_lay->setMargin(0);

    // Create the options interface
    createOptInterface();

    // create the preview
    d->previewArea = new PreviewArea(this);
    connect(d->previewArea, SIGNAL(newSelection(float, float, float, float)),
            this, SLOT(handleSelection(float, float, float, float)));
    d->previewImg = d->previewArea->getImage();

    d->zInBtn  = new QPushButton();
    d->zInBtn->setIcon(SmallIcon("zoom-in"));
    d->zInBtn->setToolTip(i18n("Zoom in preview image"));
    d->zOutBtn = new QPushButton();
    d->zOutBtn->setIcon(SmallIcon("zoom-out"));
    d->zOutBtn->setToolTip(i18n("Zoom out preview image"));
    d->zSelBtn = new QPushButton();
    d->zSelBtn->setIcon(SmallIcon("file-find"));
    d->zSelBtn->setToolTip(i18n("Zoom to selection of preview image"));
    d->zFitBtn = new QPushButton();
    d->zFitBtn->setIcon(SmallIcon("zoom-best-fit"));
    d->zFitBtn->setToolTip(i18n("Zoom to fit preview image"));

    d->progressBar = new QProgressBar();
    d->progressBar->hide();
    d->cancelBtn   = new QPushButton();
    d->cancelBtn->setIcon(SmallIcon("dialog-cancel"));
    d->cancelBtn->setToolTip(i18n("Cancel current scan operation"));
    d->cancelBtn->hide();

    d->prevBtn = new QPushButton();
    d->prevBtn->setIcon(SmallIcon("fileview-preview"));
    d->prevBtn->setToolTip(i18n("Scan preview image from device"));
    d->scanBtn = new QPushButton();
    d->scanBtn->setIcon(SmallIcon("document-save"));
    d->scanBtn->setToolTip(i18n("Scan final image from device"));

    connect(d->zInBtn, SIGNAL(clicked()),
            d->previewArea, SLOT(zoomIn()));

    connect(d->zOutBtn, SIGNAL(clicked()),
            d->previewArea, SLOT(zoomOut()));

    connect(d->zSelBtn, SIGNAL(clicked()),
            d->previewArea, SLOT(zoomSel()));

    connect(d->zFitBtn, SIGNAL(clicked()),
            d->previewArea, SLOT(zoom2Fit()));

    connect(d->scanBtn, SIGNAL(clicked()),
            this, SLOT(scanFinal()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SLOT(scanPreview()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SLOT(scanCancel()));

    QHBoxLayout *zoom_layout = new QHBoxLayout;

    pr_layout->addWidget(d->previewArea, 100);
    pr_layout->addLayout(zoom_layout, 0);

    zoom_layout->addWidget(d->zInBtn, 0);
    zoom_layout->addWidget(d->zOutBtn, 0);
    zoom_layout->addWidget(d->zSelBtn, 0);
    zoom_layout->addWidget(d->zFitBtn, 0);

    QHBoxLayout *progress_lay = new QHBoxLayout;
    progress_lay->addStretch(0);
    progress_lay->addWidget(d->progressBar, 100);
    progress_lay->addWidget(d->cancelBtn, 0);
    zoom_layout->addLayout(progress_lay, 100);

    zoom_layout->addWidget(d->prevBtn);
    zoom_layout->addWidget(d->scanBtn);

    // try to set KSaneWidget default values
    setDefaultValues();

    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();

    return true;
}

void KSaneWidget::createOptInterface()
{
    // create the container widget
    QWidget *opt_container = new QWidget(d->optArea);
    d->optArea->setWidget(opt_container);
    QVBoxLayout *opt_layout = new QVBoxLayout(opt_container);
    opt_layout->setSpacing(4);
    opt_layout->setMargin(3);

    // add the options
    // (Should Vendor and model always be visible?)
    LabeledSeparator *model_label = new LabeledSeparator(opt_container, d->modelName);
    opt_layout->addWidget(model_label);

    // basic/intermediate/All options
    QStringList strl;
    strl << "Basic" << "Advanced" << "All Options";
    LabeledCombo *opt_level = new LabeledCombo(opt_container, i18n("Option Level"), strl);
    opt_layout->addWidget(opt_level);

    // add separator line
    opt_layout->addSpacing(4);
    QFrame *line0 = new QFrame(opt_container);
    line0->setFrameShape(QFrame::HLine);
    line0->setFrameShadow(QFrame::Sunken);
    opt_layout->addWidget(line0);
    opt_layout->addSpacing(4);

    SaneOption *option;
    // Scan Source
    if ((option = d->getOption(SANE_NAME_SCAN_SOURCE)) != 0) {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // film-type
    if ((option = d->getOption(QString("film-type"))) != 0) {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    else if ((option = d->getOption(SANE_NAME_NEGATIVE)) != 0) {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Scan mode
    if ((option = d->getOption(SANE_NAME_SCAN_MODE)) != 0) {
        d->optMode = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Bitdepth
    if ((option = d->getOption(SANE_NAME_BIT_DEPTH)) != 0) {
        d->optDepth = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Threshold
    if ((option = d->getOption(SANE_NAME_THRESHOLD)) != 0) {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Resolution
    if ((option = d->getOption(SANE_NAME_SCAN_RESOLUTION)) != 0) {
        d->optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    else if ((option = d->getOption(SANE_NAME_SCAN_X_RESOLUTION)) != 0) {
        d->optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_SCAN_Y_RESOLUTION)) != 0) {
        d->optResY = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // scan area
    if ((option = d->getOption(SANE_NAME_SCAN_TL_X)) != 0) {
        d->optTl = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setTLX(float)));
    }
    if ((option = d->getOption(SANE_NAME_SCAN_TL_Y)) != 0) {
        d->optTlY = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setTLY(float)));
    }
    if ((option = d->getOption(SANE_NAME_SCAN_BR_X)) != 0) {
        d->optBrX = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setBRX(float)));
    }
    if ((option = d->getOption(SANE_NAME_SCAN_BR_Y)) != 0) {
        d->optBrY = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setBRY(float)));
    }

    // Color Options Frame
    d->colorOpts = new QWidget(opt_container);
    opt_layout->addWidget(d->colorOpts);
    QVBoxLayout *color_lay = new QVBoxLayout(d->colorOpts);
    color_lay->setSpacing(2);
    color_lay->setMargin(0);

    // add separator line
    color_lay->addSpacing(6);
    QFrame *line1 = new QFrame(d->colorOpts);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    color_lay->addWidget(line1);
    color_lay->addSpacing(2);

    if ((option = d->getOption(SANE_NAME_BRIGHTNESS)) != 0) {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_CONTRAST)) != 0) {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }

    // gamma tables
    QWidget *gamma_frm = new QWidget(d->colorOpts);
    color_lay->addWidget(gamma_frm);
    QVBoxLayout *gam_frm_l = new QVBoxLayout(gamma_frm);
    gam_frm_l->setSpacing(2);
    gam_frm_l->setMargin(0);

    if ((option = d->getOption(SANE_NAME_GAMMA_VECTOR_R)) != 0) {
        d->optGamR= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_GAMMA_VECTOR_G)) != 0) {
        d->optGamG= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_GAMMA_VECTOR_B)) != 0) {
        d->optGamB= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }

    if ((d->optGamR != 0) && (d->optGamG != 0) && (d->optGamB != 0)) {
        LabeledGamma *lgamma = new LabeledGamma(d->colorOpts,
                                  i18n(SANE_TITLE_GAMMA_VECTOR),
                                  d->optGamR->lgamma->size());
        color_lay->addWidget(lgamma);

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamR->lgamma, SLOT(setValues(int,int,int)));

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamG->lgamma, SLOT(setValues(int,int,int)));

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamB->lgamma, SLOT(setValues(int,int,int)));

        QCheckBox *split_gam_btn = new QCheckBox(i18n("Separate color intensity tables"),
                                                 opt_container);
        color_lay->addWidget(split_gam_btn);

        connect (split_gam_btn, SIGNAL(toggled(bool)),
                 gamma_frm, SLOT(setVisible(bool)));

        connect (split_gam_btn, SIGNAL(toggled(bool)),
                 lgamma, SLOT(setHidden(bool)));

        gamma_frm->hide();
    }

    if ((option = d->getOption(SANE_NAME_BLACK_LEVEL)) != 0) {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_WHITE_LEVEL)) != 0) {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }

    // Remaining (un known) Options Frame
    d->remainOpts = new QWidget(opt_container);
    opt_layout->addWidget(d->remainOpts);
    QVBoxLayout *remain_lay = new QVBoxLayout(d->remainOpts);
    remain_lay->setSpacing(2);
    remain_lay->setMargin(0);

    // add separator line
    remain_lay->addSpacing(4);
    QFrame *line2 = new QFrame(d->remainOpts);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    remain_lay->addWidget(line2);
    remain_lay->addSpacing(4);

    // add remaining parameters
    for (int i=0; i<d->optList.size(); i++)
    {
        if ((d->optList.at(i)->widget() == 0) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_TL_X) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_TL_Y) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_BR_X) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_BR_Y) &&
             (d->optList.at(i)->sw_type() != SW_GROUP))
        {
            d->optList.at(i)->createWidget(d->remainOpts);
            remain_lay->addWidget(d->optList.at(i)->widget());
        }
    }

    // connect showing/hiding finctionality
    connect (opt_level, SIGNAL(activated(int)),
             this, SLOT(opt_level_change(int)));

    // add a stretch to the end to ceep the parameters at the top
    opt_layout->addStretch();

    // encsure that you do not get a scrollbar at the bottom of the option of the options
    d->optArea->setMinimumWidth(opt_container->sizeHint().width()+20);

    // this could/should be set by saved settings.
    d->colorOpts->setVisible(false);
    d->remainOpts->setVisible(false);
}

void KSaneWidget::opt_level_change(int level)
{
    if (d->colorOpts == 0) return;
    if (d->remainOpts == 0) return;

    switch (level) {
        case 1:
            d->colorOpts->setVisible(true);
            d->remainOpts->setVisible(false);
            break;
        case 2:
            d->colorOpts->setVisible(true);
            d->remainOpts->setVisible(true);
            break;
        default:
            d->colorOpts->setVisible(false);
            d->remainOpts->setVisible(false);
    }
}

void KSaneWidget::setDefaultValues()
{
    SaneOption *option;

    // Try to get Color mode by default
    if ((option = d->getOption(SANE_NAME_SCAN_MODE)) != 0) {
        option->setValue(SANE_VALUE_SCAN_MODE_COLOR);
    }

    // Try to set 8 bit color
    if ((option = d->getOption(SANE_NAME_BIT_DEPTH)) != 0) {
        option->setValue(8);
    }

    // Try to set Scan resolution to 600 DPI
    if (d->optRes != 0) {
        d->optRes->setValue(600);
    }
}

void KSaneWidget::scheduleValReload()
{
    d->rValTmr.start(5);
}

void KSaneWidget::optReload()
{
    int i;
    //printf("Reload Options\n");

    for (i=0; i<d->optList.size(); i++) {
        d->optList.at(i)->readOption();
        // Also read the values
        d->optList.at(i)->readValue();
    }
    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();
}

void KSaneWidget::valReload()
{
    int i;
    QString tmp;
    //printf("Reload Values\n");

    for (i=0; i<d->optList.size(); i++) {
        d->optList.at(i)->readValue();
        /*
        if (d->optList.at(i)->getValue(&tmp)) {
            printf("option(%s)=%s\n",
                   qPrintable(d->optList.at(i)->name()),
                   qPrintable(tmp));
        }
        */
    }
}

void KSaneWidget::handleSelection(float tl_x, float tl_y, float br_x, float br_y) {
    float max_x, max_y;

    //printf("handleSelection0: %f %f %f %f\n", tl_x, tl_y, br_x, br_y);
    if ((d->previewImg->width()==0) || (d->previewImg->height()==0)) return;

    d->optBrX->getMaxValue(&max_x);
    d->optBrY->getMaxValue(&max_y);
    float ftl_x = tl_x*max_x;
    float ftl_y = tl_y*max_y;
    float fbr_x = br_x*max_x;
    float fbr_y = br_y*max_y;

    //printf("handleSelection1: %f %f %f %f\n", ftl_x, ftl_y, fbr_x, fbr_y);

    if (d->optTl != 0) d->optTl->setValue(ftl_x);
    if (d->optTlY != 0) d->optTlY->setValue(ftl_y);
    if (d->optBrX != 0) d->optBrX->setValue(fbr_x);
    if (d->optBrY != 0) d->optBrY->setValue(fbr_y);
}

void KSaneWidget::setTLX(float ftlx)
{
    float max, ratio;

    //kDebug() << "setTLX " << ftlx;
    d->optBrX->getMaxValue(&max);
    ratio = ftlx / max;
    //kDebug() << " -> " << ratio;
    d->previewArea->setTLX(ratio);
}

void KSaneWidget::setTLY(float ftly)
{
    float max, ratio;

    //kDebug() << "setTLY " << ftly;
    d->optBrY->getMaxValue(&max);
    ratio = ftly / max;
    //kDebug() << " -> " << ratio;
    d->previewArea->setTLY(ratio);
}

void KSaneWidget::setBRX(float fbrx)
{
    float max, ratio;

    //kDebug() << "setBRX " << fbrx;
    d->optBrX->getMaxValue(&max);
    ratio = fbrx / max;
    //kDebug() << " -> " << ratio;
    d->previewArea->setBRX(ratio);
}

void KSaneWidget::setBRY(float fbry)
{
    float max, ratio;

    //kDebug() << "setBRY " << fbry;
    d->optBrY->getMaxValue(&max);
    ratio = fbry / max;
    //kDebug() << " -> " << ratio;
    d->previewArea->setBRY(ratio);
}

void KSaneWidget::updatePreviewSize()
{
    float max_x=0, max_y=0;
    float ratio;
    int x,y;

    // check if an update is necessary
    if (d->optBrX != 0) {
        d->optBrX->getMaxValue(&max_x);
    }
    if (d->optBrY != 0) {
        d->optBrY->getMaxValue(&max_y);
    }
    if ((max_x == d->previewWidth) && (max_y == d->previewHeight)) {
        return;
    }

    d->previewWidth  = max_x;
    d->previewHeight = max_y;
    // set the scan area to the whole area
    if (d->optTl != 0) {
        d->optTl->setValue(0);
    }
    if (d->optTlY != 0) {
        d->optTlY->setValue(0);
    }

    if (d->optBrX != 0) {
        d->optBrX->setValue(max_x);
    }
    if (d->optBrY != 0) {
        d->optBrY->setValue(max_y);
    }

    // create a "scaled" image of the preview
    ratio = max_x/max_y;
    if (ratio < 1) {
        x=SCALED_PREVIEW_MAX_SIDE;
        y=(int)(SCALED_PREVIEW_MAX_SIDE/ratio);
    }
    else {
        y=SCALED_PREVIEW_MAX_SIDE;
        x=(int)(SCALED_PREVIEW_MAX_SIDE/ratio);
    }

    *d->previewImg = QImage(x, y, QImage::Format_RGB32);
    d->previewImg->fill(0xFFFFFFFF);

    // clear the selection
    d->previewArea->clearSelection();

    // update the size of the preview widget.
    d->previewArea->updateScaledImg();
}

void KSaneWidget::scanPreview()
{
    SANE_Status status;
    float max;
    int dpi;

    //kDebug() << "scanPreview";

    // store the current settings of parameters to be changed
    if (d->optDepth != 0) d->optDepth->storeCurrentData();
    if (d->optRes != 0) d->optRes->storeCurrentData();
    if (d->optResY != 0) d->optResY->storeCurrentData();
    if (d->optTl != 0) d->optTl->storeCurrentData();
    if (d->optTlY != 0) d->optTlY->storeCurrentData();
    if (d->optBrX != 0) d->optBrX->storeCurrentData();
    if (d->optBrY != 0) d->optBrY->storeCurrentData();

    // set 8 bits per color if possible
    //if (d->optDepth != 0) {
    //    d->optDepth->setValue(8);
    //}

    // select the whole area
    if (d->optTl != 0) {
        d->optTl->setValue(0);
    }
    if (d->optTlY != 0) {
        d->optTlY->setValue(0);
    }

    if (d->optBrX != 0) {
        d->optBrX->getMaxValue(&max);
        d->optBrX->setValue(max);
    }
    if (d->optBrY != 0) {
        d->optBrY->getMaxValue(&max);
        d->optBrY->setValue(max);
    }

    // set the resopution to 100 dpi and increase if necessary
    dpi = 0;
    do {
        // Increase the dpi value
        dpi += 100;
        if (d->optRes != 0) {
            d->optRes->setValue(dpi);
        }
        if (d->optResY != 0) {
            d->optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(d->saneHandle, &d->params);
        if (status != SANE_STATUS_GOOD) {
            kDebug() << "sane_get_parameters=" << sane_strstatus(status);
            return;
        }
        if (dpi > 800) break;
    }
    while ((d->params.pixels_per_line < 300) || (d->params.lines < 300));

    // execute valReload if there is a pending value reload
    while (d->rValTmr.isActive()) {
        d->rValTmr.stop();
        valReload();
    }

    // Start the scanning
    status = sane_start(d->saneHandle);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "sane_start=" << sane_strstatus(status);
        sane_cancel(d->saneHandle);
        return;
    }

    // Read image parameters
    status = sane_get_parameters(d->saneHandle, &d->params);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "sane_get_parameters=" << sane_strstatus(status);
        sane_cancel(d->saneHandle);
        return;
    }

    //kDebug() << "format =" << d->params.format;
    //kDebug() << "last_frame =" << d->params.last_frame;
    //kDebug() << "lines =" << d->params.lines;
    //kDebug() << "depth =" << d->params.depth;
    //kDebug() << "pixels_per_line =" << d->params.pixels_per_line;
    //kDebug() << "bytes_per_line =" << d->params.bytes_per_line;

    // create a new image if necessary
    if ((d->previewImg->height() != d->params.lines) ||
         (d->previewImg->width() != d->params.pixels_per_line))
    {
        *d->previewImg = QImage(d->params.pixels_per_line,
                                d->params.lines,
                                QImage::Format_RGB32);
        d->previewImg->fill(0xFFFFFFFF);
    }

    // update the size of the preview widget.
    d->previewArea->zoom2Fit();

    // update the size of the preview widget.
    d->previewArea->updateScaledImg();

    // initialize the scan variables
    d->frameSize  = d->params.lines * d->params.bytes_per_line;
    if ((d->params.format == SANE_FRAME_RED) ||
         (d->params.format == SANE_FRAME_GREEN) ||
         (d->params.format == SANE_FRAME_BLUE))
    {
        d->dataSize  = d->frameSize*3;
    }
    else {
        d->dataSize  = d->frameSize;
    }
    d->scanData.resize(0);
    d->pixel_x     = 0;
    d->pixel_y     = 0;
    d->frameRead   = 0;
    d->progress    = 0;
    d->readStatus  = READ_ON_GOING;
    d->isPreview   = true;
    d->px_c_index  = 0;
    d->frame_t_count = 0;
    d->progressBar->setValue(0);
    d->progressBar->setMaximum(d->dataSize);

    setBusy(true);

    while (d->readStatus == READ_ON_GOING) {
        processData();
    }

    d->isPreview = false;
    d->previewArea->updateScaledImg();
    setBusy(false);

    // restore the original settings of the changed parameters
    if (d->optDepth != 0) d->optDepth->restoreSavedData();
    if (d->optRes != 0) d->optRes->restoreSavedData();
    if (d->optResY != 0) d->optResY->restoreSavedData();
    if (d->optTl != 0) d->optTl->restoreSavedData();
    if (d->optTlY != 0) d->optTlY->restoreSavedData();
    if (d->optBrX != 0) d->optBrX->restoreSavedData();
    if (d->optBrY != 0) d->optBrY->restoreSavedData();

}


void KSaneWidget::scanFinal()
{
    SANE_Status status;
    float v1,v2;

    if ((d->optTl != 0) && (d->optBrX != 0)) {
        d->optTl->getValue(&v1);
        d->optBrX->getValue(&v2);
        if (v1 == v2) {
            d->optTl->setValue(0);
            d->optBrX->getMaxValue(&v2);
            d->optBrX->setValue(v2);
        }
    }

    if ((d->optTlY != 0) && (d->optBrY != 0)) {
        d->optTlY->getValue(&v1);
        d->optBrY->getValue(&v2);
        if (v1 == v2) {
            d->optTlY->setValue(0);
            d->optBrY->getMaxValue(&v2);
            d->optBrY->setValue(v2);
        }
    }

    // execute a pending value reload
    while (d->rValTmr.isActive()) {
        d->rValTmr.stop();
        valReload();
    }

    // Start the scanning
    status = sane_start(d->saneHandle);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "sane_start =" << sane_strstatus(status);
        sane_cancel(d->saneHandle);
        return;
    }

    status = sane_get_parameters(d->saneHandle, &d->params);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "sane_get_parameters =" << sane_strstatus(status);
        sane_cancel(d->saneHandle);
        return;
    }

    //kDebug() << "format =" << d->params.format;
    //kDebug() << "last_frame =" << d->params.last_frame;
    //kDebug() << "lines =" << d->params.lines;
    //kDebug() << "depth =" << d->params.depth;
    //kDebug() << "pixels_per_line =" << d->params.pixels_per_line;
    //kDebug() << "bytes_per_line =" << d->params.bytes_per_line;

    // initialize the scan variables
    d->frameSize  = d->params.lines * d->params.bytes_per_line;
    if ((d->params.format == SANE_FRAME_RED) ||
         (d->params.format == SANE_FRAME_GREEN) ||
         (d->params.format == SANE_FRAME_BLUE))
    {
        d->dataSize = d->frameSize*3;
    }
    else {
        d->dataSize = d->frameSize;
    }

    d->scanData.resize(d->dataSize);
    d->frameRead   = 0;
    d->progress    = 0;
    d->readStatus  = READ_ON_GOING;
    d->isPreview   = false;
    d->frame_t_count = 0;
    d->progressBar->setValue(0);
    d->progressBar->setMaximum(d->dataSize);

    setBusy(true);

    while (d->readStatus == READ_ON_GOING) {
        processData();
    }

    if (d->readStatus == READ_FINISHED) {
        emit imageReady(d->scanData,
                        d->params.pixels_per_line,
                        d->params.lines,
                        d->getBytesPerLines(),
                        (int)d->getImgFormat());
    }
    else if (d->readStatus == READ_ERROR) {
        KMessageBox::error(0, i18n("Scanning Failed!"));
    }

    setBusy(false);
}

#define index_rgb8_to_argb8(i)   ((i*4)/3)
#define index_rgb16_to_argb8(i)  ((i*2)/3)
#define index_rgb16_to_argb16(i) ((i*4)/3)

#define index_red8_to_argb8(i)   (i*4 + 2)
#define index_green8_to_argb8(i) (i*4 + 1)
#define index_blue8_to_argb8(i)  (i*4)

#define index_red16_to_argb8(i)   (i*2 + 2)
#define index_green16_to_argb8(i) (i*2 + 1)
#define index_blue16_to_argb8(i)  (i*2)

#define index_red16_to_argb16(i)   ((i/2)*8 + i%2)
#define index_green16_to_argb16(i) ((i/2)*8 + i%2 + 2)
#define index_blue16_to_argb16(i)  ((i/2)*8 + i%2 + 4)

//#define index_rgb8_to_rgb8(i)   (i)
//#define index_rgb16_to_rgb16(i) (i)

#define index_red8_to_rgb8(i)    (i*3)
#define index_green8_to_rgb8(i)  (i*3 + 1)
#define index_blue8_to_rgb8(i)   (i*3 + 2)

#define index_red16_to_rgb16(i)   ((i/2)*6 + i%2)
#define index_green16_to_rgb16(i) ((i/2)*6 + i%2 + 2)
#define index_blue16_to_rgb16(i)  ((i/2)*6 + i%2 + 4)

void KSaneWidget::processData()
{
    SANE_Status status = SANE_STATUS_GOOD;
    SANE_Int read_bytes = 0;

    //printf("Pre read()\n");
    status = sane_read(d->saneHandle, d->saneReadBuffer, IMG_DATA_R_SIZE, &read_bytes);
    //printf("Post read() read=%d\n", read_bytes);

    switch (status) {
        case SANE_STATUS_GOOD:
            // continue to parsing the data
            break;

        case SANE_STATUS_EOF:
            if (d->frameRead < d->frameSize) {
                kDebug() << "frameRead =" << d->frameRead
                        << ", frameSize =" << d->frameSize;
                sane_cancel(d->saneHandle);
                d->readStatus = READ_ERROR;
                return;
            }
            if (d->params.last_frame == SANE_TRUE) {
                // this is where it all ends well :)
                d->readStatus = READ_FINISHED;
                return;
            }
            else {
                // start reading next frame
                status = sane_start(d->saneHandle);
                if (status != SANE_STATUS_GOOD) {
                    kDebug() << "sane_start =" << sane_strstatus(status);
                    sane_cancel(d->saneHandle);
                    d->readStatus = READ_ERROR;
                    return;
                }
                status = sane_get_parameters(d->saneHandle, &d->params);
                if (status != SANE_STATUS_GOOD) {
                    kDebug() << "sane_get_parameters =" << sane_strstatus(status);
                    sane_cancel(d->saneHandle);
                    return;
                }
                kDebug() << "New Frame";
                d->frameRead = 0;
                d->frame_t_count++;
                break;
            }
        default:
            kDebug() << "sane_read=" << sane_strstatus(status);
            sane_cancel(d->saneHandle);
            d->readStatus = READ_ERROR;
            return;
    }

    // copy the data to the buffer
    if (d->isPreview) {
        copyToPreview((int)read_bytes);
    }
    else {
        copyToScanData((int)read_bytes);
    }

    // update progressBar
    if (d->params.lines > 0) {
        int new_progress;
        if ((d->params.format == SANE_FRAME_RED) ||
             (d->params.format == SANE_FRAME_GREEN) ||
             (d->params.format == SANE_FRAME_BLUE))
        {
            new_progress = (d->frame_t_count * d->frameSize) + d->frameRead;
        }
        else {
            new_progress = (d->frameRead);
        }
        if (abs(new_progress - d->progress) > (d->dataSize/50)) {
            d->progress = new_progress;
            if (d->isPreview) {
                d->previewArea->updateScaledImg();
            }
            if (d->progress < d->progressBar->maximum()) {
                d->progressBar->setValue(d->progress);
            }
            qApp->processEvents();
        }
    }
}

#define inc_color_index(index) { index++; if (index==3) index=0;}
#define inc_pixel(x,y,ppl) { x++; if (x>=ppl) { y++; x=0;} }

void KSaneWidget::copyToPreview(int read_bytes)
{
    int index;
    switch (d->params.format)
    {
        case SANE_FRAME_GRAY:
            if (d->params.depth == 1) {
                int i, j;
                for (i=0; i<read_bytes; i++) {
                    for (j=7; j>=0; j--) {
                        if ((d->saneReadBuffer[i] & (1<<j)) == 0) {
                            d->previewImg->setPixel(
                                    d->pixel_x,
                                    d->pixel_y,
                                    qRgb(255,255,255));
                        }
                        else {
                            d->previewImg->setPixel(
                                    d->pixel_x,
                                    d->pixel_y,
                                    qRgb(0,0,0));
                        }
                        d->pixel_x++;
                        if(d->pixel_x >= d->params.pixels_per_line) {
                            d->pixel_x = 0;
                            d->pixel_y++;
                            break;
                        }
                        if (d->pixel_y >= d->params.lines) break;
                    }
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    index = d->frameRead * 4;
                    d->previewImg->bits()[index] = d->saneReadBuffer[i];
                    d->previewImg->bits()[index + 1] = d->saneReadBuffer[i];
                    d->previewImg->bits()[index + 2] = d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    if (d->frameRead%2 == 0) {
                        index = d->frameRead * 2;
                        d->previewImg->bits()[index] = d->saneReadBuffer[i+1];
                        d->previewImg->bits()[index + 1] = d->saneReadBuffer[i+1];
                        d->previewImg->bits()[index + 2] = d->saneReadBuffer[i+1];
                    }
                    d->frameRead++;
                }
                return;
            }
            break;

        case SANE_FRAME_RGB:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->px_colors[d->px_c_index] = d->saneReadBuffer[i];
                    inc_color_index(d->px_c_index);
                    d->frameRead++;
                    if (d->px_c_index == 0) {
                        d->previewImg->setPixel(d->pixel_x,
                                           d->pixel_y,
                                           qRgb(d->px_colors[0],
                                                   d->px_colors[1],
                                                           d->px_colors[2]));
                        inc_pixel(d->pixel_x,
                                  d->pixel_y,
                                  d->params.pixels_per_line);
                    }
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    d->frameRead++;
                    if (d->frameRead%2==0) {
                        d->px_colors[d->px_c_index] = d->saneReadBuffer[i];
                        inc_color_index(d->px_c_index);
                        if (d->px_c_index == 0) {
                            d->previewImg->setPixel(d->pixel_x,
                                    d->pixel_y,
                                    qRgb(d->px_colors[0],
                                         d->px_colors[1],
                                         d->px_colors[2]));
                            inc_pixel(d->pixel_x,
                                      d->pixel_y,
                                      d->params.pixels_per_line);
                        }
                    }
                }
                return;
            }
            break;

        case SANE_FRAME_RED:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->previewImg->bits()[index_red8_to_argb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    if (d->frameRead%2 == 0) {
                        d->previewImg->bits()[index_red16_to_argb8(d->frameRead)] =
                                d->saneReadBuffer[i+1];
                    }
                    d->frameRead++;
                }
                return;
            }
            break;

        case SANE_FRAME_GREEN:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->previewImg->bits()[index_green8_to_argb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    if (d->frameRead%2 == 0) {
                        d->previewImg->bits()[index_green16_to_argb8(d->frameRead)] =
                                d->saneReadBuffer[i+1];
                    }
                    d->frameRead++;
                }
                return;
            }
            break;

        case SANE_FRAME_BLUE:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->previewImg->bits()[index_blue8_to_argb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    if (d->frameRead%2 == 0) {
                        d->previewImg->bits()[index_blue16_to_argb8(d->frameRead)] =
                                d->saneReadBuffer[i+1];
                    }
                    d->frameRead++;
                }
                return;
            }
            break;
    }
    kDebug() << "Format" << d->params.format
            << "and depth" << d->params.format
            << "is not yet suppoeted!";
    sane_cancel(d->saneHandle);
    d->readStatus = READ_ERROR;
    return;
}

void KSaneWidget::copyToScanData(int read_bytes)
{
    switch (d->params.format)
    {
        case SANE_FRAME_GRAY:
            for (int i=0; i<read_bytes; i++) {
                d->scanData[d->frameRead] =
                        d->saneReadBuffer[i];
                d->frameRead++;
            }
            return;
        case SANE_FRAME_RGB:
            if (d->params.depth == 1) {
                break;
            }
            for (int i=0; i<read_bytes; i++) {
                d->scanData[d->frameRead] =
                        d->saneReadBuffer[i];
                d->frameRead++;
            }
            return;

        case SANE_FRAME_RED:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_red8_to_rgb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_red16_to_rgb16(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            break;

        case SANE_FRAME_GREEN:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_green8_to_rgb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_green16_to_rgb16(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            break;

        case SANE_FRAME_BLUE:
            if (d->params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_blue8_to_rgb8(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            else if (d->params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    d->scanData[index_blue16_to_rgb16(d->frameRead)] =
                            d->saneReadBuffer[i];
                    d->frameRead++;
                }
                return;
            }
            break;
    }

    kDebug() << "Format" << d->params.format
            << "and depth" << d->params.format
            << "is not yet suppoeted!";
    sane_cancel(d->saneHandle);
    d->readStatus = READ_ERROR;
    return;
}



bool KSaneWidget::makeQImage(const QByteArray &data,
                              int width,
                              int height,
                              int bytes_per_line,
                              ImageFormat format,
                              QImage &img)
{
    int j=0;
    int pixel_x = 0;
    int pixel_y = 0;

    switch (format)
    {
        case FormatBlackWhite:
            img = QImage((uchar*)data.data(),
                          width,
                          height,
                          bytes_per_line,
                          QImage::Format_Mono);
            for (int i=0; i<img.height()*img.bytesPerLine(); i++) {
                img.bits()[i] = ~img.bits()[i];
            }
            return true;

        case FormatGrayScale8:
            img = QImage(width,
                         height,
                         QImage::Format_RGB32);
            j=0;
            for (int i=0; i<data.size(); i++) {
                img.bits()[j+0] = data.data()[i];
                img.bits()[j+1] = data.data()[i];
                img.bits()[j+2] = data.data()[i];
                j+=4;
            }
            return true;

        case FormatGrayScale16:
            img = QImage(width,
                         height,
                         QImage::Format_RGB32);
            j=0;
            for (int i=1; i<data.size(); i+=2) {
                img.bits()[j+0] = data.data()[i];
                img.bits()[j+1] = data.data()[i];
                img.bits()[j+2] = data.data()[i];
                j+=4;
            }
            KMessageBox::sorry(0, i18n("The image data contains 16 bits per color, "
                                         "but QImage support only 8 bits per color. "
                                         "The image data will be truncated to 8 bits per color."));
            return true;

        case FormatRGB_8_C:
            pixel_x = 0;
            pixel_y = 0;

            img = QImage(width,
                         height,
                         QImage::Format_RGB32);

            for (int i=0; i<data.size(); i+=3) {
                img.setPixel(pixel_x,
                             pixel_y,
                             qRgb(data[i],
                                  data[i+1],
                                  data[i+2]));

                inc_pixel(pixel_x, pixel_y, width);
            }
            return true;

        case FormatRGB_16_C:
            pixel_x = 0;
            pixel_y = 0;

            img = QImage(width,
                         height,
                         QImage::Format_RGB32);

            for (int i=1; i<data.size(); i+=6) {
                img.setPixel(pixel_x,
                             pixel_y,
                             qRgb(data[i],
                                  data[i+2],
                                  data[i+4]));

                inc_pixel(pixel_x, pixel_y, width);
            }
            KMessageBox::sorry(0, i18n("The image data contains 16 bits per color, "
                                         "but QImage support only 8 bits per color. "
                                         "The image data will be truncated to 8 bits per color."));
            return true;

        case FormatNone:
            break;
    }
    kDebug() << "Unsupported conversion";
    return false;
}

void KSaneWidget::scanCancel()
{
    sane_cancel(d->saneHandle);
    d->readStatus = READ_CANCEL;
}


void KSaneWidget::setBusy(bool busy)
{
    d->optArea->setDisabled(busy);
    d->previewArea->setDisabled(busy);
    d->zInBtn->setDisabled(busy);
    d->zOutBtn->setDisabled(busy);
    d->zSelBtn->setDisabled(busy);
    d->zFitBtn->setDisabled(busy);
    d->prevBtn->setDisabled(busy);
    d->scanBtn->setDisabled(busy);

    if (busy) {
        d->progressBar->show();
        d->cancelBtn->show();
    }
    else {
        d->progressBar->hide();
        d->cancelBtn->hide();
    }

    d->progressBar->setDisabled(!busy);
    d->cancelBtn->setDisabled(!busy);
}

}  // NameSpace KSaneIface
