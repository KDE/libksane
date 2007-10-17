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
#include <QPushButton>
#include <QImage>
#include <QTimer>
#include <QScrollArea>
#include <QList>
#include <QProgressBar>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
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

namespace KSaneIface
{

class SaneWidgetPriv
{
public:

    SaneWidgetPriv()
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
        preview       = 0;
        prImg         = 0;
        previewWidth  = 0;
        previewHeight = 0;
        pxCIndex      = 0;
        progress      = 0;
        pixelX        = 0;
        pixelY        = 0;
        readStatus    = READ_NOT_READING;
        scanImg       = 0;
        progressBar   = 0;
        theImg        = QImage(10, 10, QImage::Format_RGB32);
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
    PreviewArea        *preview;
    QImage             *prImg;
    float               previewWidth;
    float               previewHeight;

    // general scanning
    unsigned int        pxCIndex;
    int                 progress;
    int                 pixelX;
    int                 pixelY;
    SANE_Parameters     params;
    SANE_Byte           imgData[IMG_DATA_R_SIZE];
    SANE_Byte           pxColors[3];
    ReadStatus          readStatus;
    QImage             *scanImg;
    QImage              theImg;
};

SaneWidget::SaneWidget(QWidget* parent)
          : QWidget(parent)
{
    d = new SaneWidgetPriv;

    SANE_Int    version;
    SANE_Status status;
    status = sane_init(&version, 0);
    if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "libksane: sane_init() failed("
                 << sane_strstatus(status) << ")" << endl;
    }
    else
    {
        kDebug() << "Sane Version = "
                 << SANE_VERSION_MAJOR(version) << "."
                 << SANE_VERSION_MINOR(version) << "."
                 << SANE_VERSION_BUILD(version) << endl;
    }
    d->rValTmr.setSingleShot(true);

    connect (&d->rValTmr, SIGNAL(timeout()),
             this, SLOT(valReload()));
}

SaneWidget::~SaneWidget()
{
    d->optList.clear();
    sane_exit();
    delete d;
}

QString SaneWidget::vendor() const
{
    return d->vendor;
}

QString SaneWidget::make() const
{
    return d->vendor;
}

QString SaneWidget::model() const
{
    return d->model;
}

QString SaneWidget::selectDevice(QWidget* parent)
{
    int                 i=0;
    int                 num_scaners;
    QStringList         dev_name_list;
    QString             tmp;
    SANE_Status         status;
    SANE_Device const **dev_list;

    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0)
    {
        kDebug() << "i="       << i << " "
                 << "name='"   << dev_list[i]->name   << "' "
                 << "vendor='" << dev_list[i]->vendor << "' "
                 << "model='"  << dev_list[i]->model  << "' "
                 << "type='"   << dev_list[i]->type   << "' " << endl;
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

    if (dev_name_list.isEmpty())
    {
        KMessageBox::sorry(0, i18n("No scanner device has been found."));
        return QString();
    }

    RadioSelect sel;
    sel.setWindowTitle(qApp->applicationName());
    i = sel.getSelectedIndex(parent, QString("Select Scanner"), dev_name_list, 0);
    kDebug() << "i=" << i << endl;

    if (i == num_scaners)
    {
        return QString("test:0");
    }

    if ((i < 0) || (i >= num_scaners))
    {
        return QString();
    }

    return QString(dev_list[i]->name);
}

bool SaneWidget::openDevice(const QString &device_name)
{
    int                            i=0;
    const SANE_Option_Descriptor  *num_option_d;
    SANE_Status                    status;
    SANE_Word                      num_sane_options;
    SANE_Int                       res;
    SANE_Device const            **dev_list;

    // get the device list to get the vendor and model info
    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0)
    {
        if (QString(dev_list[i]->name) == device_name)
        {
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
        kDebug() << "openDevice: device '" << qPrintable(device_name)
                 << "' not found" << endl;
        return false;
#endif
    }

    // Try to open the device
    if (sane_open(device_name.toLatin1(), &d->saneHandle) != SANE_STATUS_GOOD)
    {
        kDebug() << "openDevice: sane_open(\"" << qPrintable(device_name)
                 << "\", &handle) failed!" << endl;
        return false;
    }
    //printf("openDevice: sane_open(\"%s\", &handle) == SANE_STATUS_GOOD\n", qPrintable(device_name));

    // Read the options (start with option 0 the number of parameters)
    num_option_d = sane_get_option_descriptor(d->saneHandle, 0);
    if (num_option_d == 0)
    {
        return false;
    }
    QVarLengthArray<char> data(num_option_d->size);
    status = sane_control_option(d->saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD)
    {
        return false;
    }
    num_sane_options = *reinterpret_cast<SANE_Word*>(data.data());

    // read the rest of the options
    for (i=1; i<num_sane_options; i++)
    {
        d->optList.append(new SaneOption(d->saneHandle, i));
    }

    // do the connections of the option parameters
    for (i=1; i<d->optList.size(); i++)
    {
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
    d->preview = new PreviewArea(this);
    connect(d->preview, SIGNAL(newSelection(float, float, float, float)),
            this, SLOT(handleSelection(float, float, float, float)));
    d->prImg = d->preview->getImage();

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
            d->preview, SLOT(zoomIn()));

    connect(d->zOutBtn, SIGNAL(clicked()),
            d->preview, SLOT(zoomOut()));

    connect(d->zSelBtn, SIGNAL(clicked()),
            d->preview, SLOT(zoomSel()));

    connect(d->zFitBtn, SIGNAL(clicked()),
            d->preview, SLOT(zoom2Fit()));

    connect(d->scanBtn, SIGNAL(clicked()),
            this, SLOT(scanFinal()));

    connect(d->prevBtn, SIGNAL(clicked()),
            this, SLOT(scanPreview()));

    connect(d->cancelBtn, SIGNAL(clicked()),
            this, SLOT(scanCancel()));

    QHBoxLayout *zoom_layout = new QHBoxLayout;

    pr_layout->addWidget(d->preview, 100);
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

    // try to set SaneWidget default values
    setDefaultValues();

    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();

    //d->preview->zoom2Fit();

    return true;
}

void SaneWidget::createOptInterface()
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
    if ((option = getOption(SANE_NAME_SCAN_SOURCE)) != 0)
    {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // film-type
    if ((option = getOption(QString("film-type"))) != 0)
    {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    else if ((option = getOption(SANE_NAME_NEGATIVE)) != 0)
    {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Scan mode
    if ((option = getOption(SANE_NAME_SCAN_MODE)) != 0)
    {
        d->optMode = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Bitdepth
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0)
    {
        d->optDepth = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Threshold
    if ((option = getOption(SANE_NAME_THRESHOLD)) != 0)
    {
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Resolution
    if ((option = getOption(SANE_NAME_SCAN_RESOLUTION)) != 0)
    {
        d->optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    else if ((option = getOption(SANE_NAME_SCAN_X_RESOLUTION)) != 0)
    {
        d->optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_SCAN_Y_RESOLUTION)) != 0)
    {
        d->optResY = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // scan area
    if ((option = getOption(SANE_NAME_SCAN_TL_X)) != 0)
    {
        d->optTl = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setTLX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_TL_Y)) != 0)
    {
        d->optTlY = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setTLY(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_X)) != 0)
    {
        d->optBrX = option;
        connect (option, SIGNAL(fValueRead(float)),
                 this, SLOT(setBRX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_Y)) != 0)
    {
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

    if ((option = getOption(SANE_NAME_BRIGHTNESS)) != 0)
    {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_CONTRAST)) != 0)
    {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }

    // gamma tables
    QWidget *gamma_frm = new QWidget(d->colorOpts);
    color_lay->addWidget(gamma_frm);
    QVBoxLayout *gam_frm_l = new QVBoxLayout(gamma_frm);
    gam_frm_l->setSpacing(2);
    gam_frm_l->setMargin(0);

    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_R)) != 0)
    {
        d->optGamR= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_G)) != 0)
    {
        d->optGamG= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_B)) != 0)
    {
        d->optGamB= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }

    if ((d->optGamR != 0) && (d->optGamG != 0) && (d->optGamB != 0))
    {
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

    if ((option = getOption(SANE_NAME_BLACK_LEVEL)) != 0)
    {
        option->createWidget(d->colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_WHITE_LEVEL)) != 0)
    {
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

void SaneWidget::opt_level_change(int level)
{
    if (d->colorOpts == 0) return;
    if (d->remainOpts == 0) return;

    switch (level)
    {
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

void SaneWidget::setDefaultValues()
{
    SaneOption *option;

    // Try to get Color mode by default
    if ((option = getOption(SANE_NAME_SCAN_MODE)) != 0)
    {
        option->setValue(SANE_VALUE_SCAN_MODE_COLOR);
    }

    // Try to set 8 bit color
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0)
    {
        option->setValue(8);
    }

    // Try to set Scan resolution to 600 DPI
    if (d->optRes != 0)
    {
        d->optRes->setValue(600);
    }
}

void SaneWidget::scheduleValReload()
{
    d->rValTmr.start(5);
}

void SaneWidget::optReload()
{
    int i;
    //printf("Reload Options\n");

    for (i=0; i<d->optList.size(); i++)
    {
        d->optList.at(i)->readOption();
        // Also read the values
        d->optList.at(i)->readValue();
    }
    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();
}

void SaneWidget::valReload()
{
    int i;
    QString tmp;
    //printf("Reload Values\n");

    for (i=0; i<d->optList.size(); i++)
    {
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

SaneOption *SaneWidget::getOption(const QString &name)
{
    int i;

    for (i=0; i<d->optList.size(); i++)
    {
        if (d->optList.at(i)->name() == name)
        {
            return d->optList.at(i);
        }
    }
    return 0;
}

void SaneWidget::handleSelection(float tl_x, float tl_y, float br_x, float br_y) {
    float max_x, max_y;

    //printf("handleSelection0: %f %f %f %f\n", tl_x, tl_y, br_x, br_y);
    if ((d->prImg->width()==0) || (d->prImg->height()==0)) return;

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

void SaneWidget::setTLX(float ftlx)
{
    float max, ratio;

    //std::cout << "setTLX " << ftlx;
    d->optBrX->getMaxValue(&max);
    ratio = ftlx / max;
    //std::cout << " -> " << ratio << std::endl;
    d->preview->setTLX(ratio);
}

void SaneWidget::setTLY(float ftly)
{
    float max, ratio;

    //std::cout << "setTLY " << ftly;
    d->optBrY->getMaxValue(&max);
    ratio = ftly / max;
    //std::cout << " -> " << ratio << std::endl;
    d->preview->setTLY(ratio);
}

void SaneWidget::setBRX(float fbrx)
{
    float max, ratio;

    //std::cout << "setBRX " << fbrx;
    d->optBrX->getMaxValue(&max);
    ratio = fbrx / max;
    //std::cout << " -> " << ratio << std::endl;
    d->preview->setBRX(ratio);
}

void SaneWidget::setBRY(float fbry)
{
    float max, ratio;

    //std::cout << "setBRY " << fbry;
    d->optBrY->getMaxValue(&max);
    ratio = fbry / max;
    //std::cout << " -> " << ratio << std::endl;
    d->preview->setBRY(ratio);
}

void SaneWidget::updatePreviewSize()
{
    SANE_Status status;
    int i, j, dpi;
    float max_x=0, max_y=0;

    // check if an update is necessary
    if (d->optBrX != 0)
    {
        d->optBrX->getMaxValue(&max_x);
    }
    if (d->optBrY != 0)
    {
        d->optBrY->getMaxValue(&max_y);
    }
    if ((max_x == d->previewWidth) && (max_y == d->previewHeight))
    {
        return;
    }

    d->previewWidth  = max_x;
    d->previewHeight = max_y;
    // set the scan area to the whole area
    if (d->optTl != 0)
    {
        d->optTl->setValue(0);
    }
    if (d->optTlY != 0)
    {
        d->optTlY->setValue(0);
    }

    if (d->optBrX != 0)
    {
        d->optBrX->setValue(max_x);
    }
    if (d->optBrY != 0)
    {
        d->optBrY->setValue(max_y);
    }

    // set the resopution to 100 dpi and increase if necessary
    dpi = 0;
    do
    {
        // Increase the dpi value
        dpi += 100;
        if (d->optRes != 0)
        {
            d->optRes->setValue(dpi);
        }
        if (d->optResY != 0)
        {
            d->optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(d->saneHandle, &d->params);
        if (status != SANE_STATUS_GOOD)
        {
            kDebug() << "ERROR: status="
                     << sane_strstatus(status) << endl;
            return;
        }
        //printf("dpi = %d\n", dpi);
        //printf("lines = %d\n", d->params.lines);
        //printf("pixels_per_line = %d\n", d->params.pixels_per_line);
        if (dpi > 800) break;
    }
    while ((d->params.pixels_per_line < 300) || (d->params.lines < 300));


    if ((d->prImg->width() != d->params.pixels_per_line) || (d->prImg->height() != d->params.lines))
    {
        *d->prImg = QImage(d->params.pixels_per_line, d->params.lines, QImage::Format_RGB32);
        for (i=0; i<d->prImg->height(); i++)
        {
            for (j=0; j<d->prImg->width(); j++)
            {
                d->prImg->setPixel(j, i, qRgb(255,255,255));
            }
        }

        // clear the selection
        d->preview->clearSelection();

        // update the size of the preview widget.
        d->preview->updateScaledImg();
        //d->preview->resize(d->preview->sizeHint());
    }
}

void SaneWidget::scanPreview()
{
    SANE_Status status;
    float max;
    int i, j, dpi;

    //std::cout << "scanPreview" << std::endl;

    // store the current settings of parameters to be changed
    if (d->optDepth != 0) d->optDepth->storeCurrentData();
    if (d->optRes != 0) d->optRes->storeCurrentData();
    if (d->optResY != 0) d->optResY->storeCurrentData();
    if (d->optTl != 0) d->optTl->storeCurrentData();
    if (d->optTlY != 0) d->optTlY->storeCurrentData();
    if (d->optBrX != 0) d->optBrX->storeCurrentData();
    if (d->optBrY != 0) d->optBrY->storeCurrentData();

    // set 8 bits per color if possible
    if (d->optDepth != 0)
    {
        d->optDepth->setValue(8);
    }

    // select the whole area
    if (d->optTl != 0)
    {
        d->optTl->setValue(0);
    }
    if (d->optTlY != 0)
    {
        d->optTlY->setValue(0);
    }

    if (d->optBrX != 0)
    {
        d->optBrX->getMaxValue(&max);
        d->optBrX->setValue(max);
    }
    if (d->optBrY != 0)
    {
        d->optBrY->getMaxValue(&max);
        d->optBrY->setValue(max);
    }

    // set the resopution to 100 dpi and increase if necessary
    dpi = 0;
    do
    {
        // Increase the dpi value
        dpi += 100;
        if (d->optRes != 0)
        {
            d->optRes->setValue(dpi);
        }
        if (d->optResY != 0)
        {
            d->optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(d->saneHandle, &d->params);
        if (status != SANE_STATUS_GOOD)
        {
            kDebug() << "ERROR: status="
                     << sane_strstatus(status) << endl;
            return;
        }
        //printf("dpi = %d\n", dpi);
        //printf("lines = %d\n", d->params.lines);
        //printf("pixels_per_line = %d\n", d->params.pixels_per_line);
        if (dpi > 800) break;
    }
    while ((d->params.pixels_per_line < 300) || (d->params.lines < 300));

    // execute valReload if there is a pending value reload
    while (d->rValTmr.isActive())
    {
        d->rValTmr.stop();
        valReload();
    }

    // Start the scanning
    status = sane_start(d->saneHandle);
    if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "sane_start ERROR: status="
                 << sane_strstatus(status) << endl;

        sane_cancel(d->saneHandle);
        return;
    }

    status = sane_get_parameters(d->saneHandle, &d->params);
    if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "sane_get_parameters ERROR: status="
                 << sane_strstatus(status) << endl;
        sane_cancel(d->saneHandle);
        return;
    }

    //printf("format = %d\n", d->params.format);
    //printf("last_frame = %d\n", d->params.last_frame);
    //printf("lines = %d\n", d->params.lines);
    //printf("depth = %d\n", d->params.depth);
    //printf("pixels_per_line = %d\n", d->params.pixels_per_line);
    //printf("bytes_per_line = %d\n", d->params.bytes_per_line);

    // create a new image if necessary
    //(This image should be small so who cares about waisted memory :)
    // FIXME optimize size
    d->scanImg = d->prImg;
    if ((d->prImg->height() != d->params.lines) ||
         (d->prImg->width() != d->params.pixels_per_line))
    {
        *d->prImg = QImage(d->params.pixels_per_line, d->params.lines, QImage::Format_RGB32);
    }

    // clear the old image
    for (i=0; i<d->prImg->height(); i++)
    {
        for (j=0; j<d->prImg->width(); j++)
        {
            d->prImg->setPixel(j, i, qRgb(255,255,255));
        }
    }

    // update the size of the preview widget.
    d->preview->zoom2Fit();

    d->readStatus = READ_ON_GOING;
    d->pixelX     = 0;
    d->pixelY     = 0;
    d->pxCIndex   = 0;

    setBusy(true);

    while (d->readStatus == READ_ON_GOING)
    {
        processData();
    }

    d->preview->updateScaledImg();

    // restore the original settings of the changed parameters
    if (d->optDepth != 0) d->optDepth->restoreSavedData();
    if (d->optRes != 0) d->optRes->restoreSavedData();
    if (d->optResY != 0) d->optResY->restoreSavedData();
    if (d->optTl != 0) d->optTl->restoreSavedData();
    if (d->optTlY != 0) d->optTlY->restoreSavedData();
    if (d->optBrX != 0) d->optBrX->restoreSavedData();
    if (d->optBrY != 0) d->optBrY->restoreSavedData();

    setBusy(false);
}

void SaneWidget::scanFinal()
{
    SANE_Status status;
    float v1,v2;

    //std::cout << "scanFinal" << std::endl;

    if ((d->optTl != 0) && (d->optBrX != 0))
    {
        d->optTl->getValue(&v1);
        d->optBrX->getValue(&v2);
        if (v1 == v2)
        {
            d->optTl->setValue(0);
            d->optBrX->getMaxValue(&v2);
            d->optBrX->setValue(v2);
        }
    }

    if ((d->optTlY != 0) && (d->optBrY != 0))
    {
        d->optTlY->getValue(&v1);
        d->optBrY->getValue(&v2);
        if (v1 == v2)
        {
            d->optTlY->setValue(0);
            d->optBrY->getMaxValue(&v2);
            d->optBrY->setValue(v2);
        }
    }

    // execute a pending value reload
    while (d->rValTmr.isActive())
    {
        d->rValTmr.stop();
        valReload();
    }

    // Start the scanning
    status = sane_start(d->saneHandle);
    if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "sane_start ERROR: status="
                 << sane_strstatus(status) << endl;
        sane_cancel(d->saneHandle);
        return;
    }
    //printf("start OK\n");

    status = sane_get_parameters(d->saneHandle, &d->params);
    if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "sane_get_parameters ERROR: status="
                 << sane_strstatus(status) << endl;
        sane_cancel(d->saneHandle);
        return;
    }

    //printf("format = %d\n", d->params.format);
    //printf("last_frame = %d\n", d->params.last_frame);
    //printf("lines = %d\n", d->params.lines);
    //printf("depth = %d\n", d->params.depth);
    //printf("pixels_per_line = %d\n", d->params.pixels_per_line);
    //printf("bytes_per_line = %d\n", d->params.bytes_per_line);

    // create a new image
    //(This image should be small so who cares about waisted memory :)
    // FIXME optimize size
    d->scanImg  = &d->theImg;
    *d->scanImg = QImage(d->params.pixels_per_line, d->params.lines, QImage::Format_RGB32);

    d->readStatus = READ_ON_GOING;
    d->pixelX = 0;
    d->pixelY = 0;
    d->pxCIndex = 0;

    setBusy(true);

    while (d->readStatus == READ_ON_GOING)
    {
        processData();
    }
    if (d->readStatus != READ_FINISHED)
    {
        emit scanFaild();
    }
    setBusy(false);
}

void SaneWidget::processData()
{
    SANE_Status status = SANE_STATUS_GOOD;
    SANE_Int read_bytes = 0;
    int i, j;

    //printf("Pre read()\n");
    status = sane_read(d->saneHandle, d->imgData, IMG_DATA_R_SIZE, &read_bytes);
    //printf("Post read() read=%d\n", read_bytes);

    if (status == SANE_STATUS_EOF)
    {
        //printf("Read finished read_bytes=%d\n", read_bytes);
        if (d->pixelY < d->params.lines)
        {
            kDebug() << "d->pixelY(" << d->pixelY
                     << ") < d->params.lines(" << d->params.lines << ")" << endl;
            sleep(1);
            //sane_cancel(d->saneHandle);
        }
        if (d->params.last_frame == SANE_TRUE)
        {
            // this is where it all ends well :)
            d->readStatus = READ_FINISHED;
            if (d->scanImg == &d->theImg)
            {
                d->progressBar->hide();
                d->cancelBtn->hide();
                emit imageReady();
            }
            return;
        }
        else
        {
            sane_start(d->saneHandle);
            if (status != SANE_STATUS_GOOD)
            {
                kDebug() << "sane_start ERROR: status="
                         << sane_strstatus(status) << endl;
                sane_cancel(d->saneHandle);
                d->readStatus = READ_ERROR;
                return;
            }
        }
    }
    else if (status != SANE_STATUS_GOOD)
    {
        kDebug() << "Reading error, status="
                 << sane_strstatus(status) << endl;
        sane_cancel(d->saneHandle);
        d->readStatus = READ_ERROR;
        return;
    }

    switch (d->params.format)
    {
        case SANE_FRAME_RGB:
            if (d->params.depth == 8)
            {
                // go through the data
                for (i=0; i<read_bytes; i++)
                {
                    if (d->pixelY >= d->params.lines)
                    {
                        kDebug() << "processData: reached image height before EOF"
                                 << endl;
                        sane_cancel(d->saneHandle);
                        d->readStatus = READ_ERROR;
                        return;
                    }
                    d->pxColors[d->pxCIndex] = d->imgData[i];
                    d->pxCIndex++;
                    if (d->pxCIndex >= 3) d->pxCIndex = 0;

                    if (d->pxCIndex == 0)
                    {
                        d->scanImg->setPixel(d->pixelX, d->pixelY,
                                           qRgb(d->pxColors[0],
                                                d->pxColors[1],
                                                d->pxColors[2]));
                        d->pixelX++;
                        if (d->pixelX >= d->params.pixels_per_line)
                        {
                            d->pixelX = 0;
                            d->pixelY++;
                        }
                    }
                }
            }
            else
            {
                kDebug() << "Only 8-bit colors are currently supported!"
                         << endl;
                sane_cancel(d->saneHandle);
                d->readStatus = READ_ERROR;
                return;
            }
            break;

        case SANE_FRAME_GRAY:
            if (d->params.depth == 8)
            {
                for (i=0; i<read_bytes; i++)
                {
                    if (d->pixelY >= d->params.lines)
                    {
                        kDebug() << "reached image height before EOF" << endl;
                        sane_cancel(d->saneHandle);
                        d->readStatus = READ_ERROR;
                        return;
                    }
                    d->scanImg->setPixel(d->pixelX, d->pixelY,
                                       qRgb(d->imgData[i],
                                            d->imgData[i],
                                            d->imgData[i]));
                    d->pixelX++;
                    if (d->pixelX >= d->params.pixels_per_line)
                    {
                        d->pixelX = 0;
                        d->pixelY++;
                    }
                }
            }
            else if (d->params.depth == 1)
            {
                for (i=0; i<read_bytes; i++)
                {
                    if (d->pixelY >= d->params.lines)
                    {
                        kDebug() << "reached image height before EOF" << endl;
                        sane_cancel(d->saneHandle);
                        d->readStatus = READ_ERROR;
                        return;
                    }
                    for (j=7; j>=0; j--)
                    {
                        if ((d->imgData[i] & (1<<j)) == 0)
                        {
                            d->scanImg->setPixel(d->pixelX, d->pixelY, qRgb(255,255,255));
                        }
                        else
                        {
                            d->scanImg->setPixel(d->pixelX, d->pixelY, qRgb(0,0,0));
                        }
                        d->pixelX++;
                        if(d->pixelX >= d->params.pixels_per_line)
                        {
                            d->pixelX = 0;
                            d->pixelY++;
                            break;
                        }
                        if (d->pixelY >= d->params.lines) break;
                    }
                }
            }
            else
            {
                kDebug() << "Only 1 and 8-bit colors are supported "
                            "for grayscale!" << endl;
                sane_cancel(d->saneHandle);
                d->readStatus = READ_ERROR;
                return;
            }
            break;

        /*
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
            for (i=0; i<read_bytes; i++) {
                printf("%d\n", pr_img_data[i]);
                if (d->pixelY >= d->params.lines) {
                    printf("reached image height before EOF\n");
                    sane_cancel(d->saneHandle);
                    d->readStatus = READ_ERROR;
                    return;
                }

                color = d->prImg->pixel(d->pixelX, d->pixelY);
                red   = qRed(color);
                green = qGreen(color);
                blue  = qBlue(color);

                switch(d->params.format)
                {
                    case SANE_FRAME_RED :
                        newColor = qRgb(pr_img_data[i], green, blue);
                        break;
                    case SANE_FRAME_GREEN :
                        newColor = qRgb(red, pr_img_data[i], blue);
                        break;
                    case SANE_FRAME_BLUE :
                        newColor = qRgb(red , green, pr_img_data[i]);
                        break;
                    default:
                        // we cannot come here but just in any case :)
                        newColor = qRgb(0,0,0);
                        break;
                }
                d->scanImg->setPixel(d->pixelX, d->pixelY, newColor);
                d->pixelX++;
                if(d->pixelX >= d->params.pixels_per_line) {
                    d->pixelX = 0;
                    d->pixelY++;
                    break;
                }
            }
            break;
        */
        default :
            kDebug() << "This frame format ( " << d->params.format
                     << ") is not yet suppoeted!" << endl;
            sane_cancel(d->saneHandle);
            d->readStatus = READ_ERROR;
            return;
    }
    if (d->params.lines > 0)
    {
        int new_progress = (int)( ((double)PROGRESS_MAX / d->params.lines) * d->pixelY);
        if (abs (new_progress - d->progress) > 5)
        {
            d->progress = new_progress;
            if (d->scanImg == d->prImg)
            {
                d->preview->updateScaledImg();
            }
            if (d->progress < PROGRESS_MAX)
            {
                d->progressBar->setValue(d->progress);
            }
            qApp->processEvents();
        }
    }
}

QImage *SaneWidget::getFinalImage()
{
    return &d->theImg;
}

void SaneWidget::scanCancel()
{
    sane_cancel(d->saneHandle);
    d->readStatus = READ_CANCEL;
}


void SaneWidget::setBusy(bool busy)
{
    d->optArea->setDisabled(busy);
    d->preview->setDisabled(busy);
    d->zInBtn->setDisabled(busy);
    d->zOutBtn->setDisabled(busy);
    d->zSelBtn->setDisabled(busy);
    d->zFitBtn->setDisabled(busy);
    d->prevBtn->setDisabled(busy);
    d->scanBtn->setDisabled(busy);

    if (busy)
    {
        d->progressBar->show();
        d->cancelBtn->show();
    }
    else
    {
        d->progressBar->hide();
        d->cancelBtn->hide();
    }

    d->progressBar->setDisabled(!busy);
    d->cancelBtn->setDisabled(!busy);
}

}  // NameSpace KSaneIface
