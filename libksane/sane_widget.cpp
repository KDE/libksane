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

#define ENABLE_DEBUG

// C++ includes.

#include <iostream>

// Sane includes.

extern "C"
{
#include <sys/ioctl.h>
#include <sane/saneopts.h>
}

// Qt includes.

#include <QEventLoop>
#include <QApplication>
#include <QVarLengthArray>
#include <QComboBox>
#include <QPushButton>

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
#include "sane_widget.h"
#include "sane_widget.moc"

namespace KSaneIface
{

SaneWidget::SaneWidget(QWidget* parent)
          : QWidget(parent)
{
    m_optArea       = 0;
    m_optMode       = 0;
    m_optDepth      = 0;
    m_optRes        = 0;
    m_optResY       = 0;
    m_optTl         = 0;
    m_optTlY        = 0;
    m_optBrX        = 0;
    m_optBrY        = 0;
    m_optGamR       = 0;
    m_optGamG       = 0;
    m_optGamB       = 0;
    m_colorOpts     = 0;
    m_remainOpts    = 0;
    m_scanBtn       = 0;
    m_prevBtn       = 0;
    m_zInBtn        = 0;
    m_zOutBtn       = 0;
    m_zSelBtn       = 0;
    m_zFitBtn       = 0;
    m_preview       = 0;
    m_prImg         = 0;
    m_previewWidth  = 0;
    m_previewHeight = 0;

    read_status   = READ_NOT_READING;
    px_c_index    = 0;
    the_img       = QImage(10, 10, QImage::Format_RGB32);

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
    m_rValTmr.setSingleShot(true);

    connect (&m_rValTmr, SIGNAL(timeout()), 
             this, SLOT(valReload()));
}

SaneWidget::~SaneWidget()
{
    m_optList.clear();
    sane_exit();
}

QString SaneWidget::make() const
{
    return m_make;
}

QString SaneWidget::model() const
{
    return m_model;
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
                 << "name='"   << dev_list[i]->name << "' "
                 << "vendor='" << dev_list[i]->vendor << "' "
                 << "model='"  << dev_list[i]->model << "' "
                 << "type='"   << dev_list[i]->type << "' " << endl;
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
            m_modelName = QString(dev_list[i]->vendor) + " " + QString(dev_list[i]->model);
            m_make      = QString(dev_list[i]->vendor);
            m_model     = QString(dev_list[i]->model);
            break;
        }
        i++;
    }

    if (dev_list[i] == 0) 
    {
#ifdef ENABLE_DEBUG
        m_modelName = i18n("Test Scanner");
#else
        kDebug() << "openDevice: device '" << qPrintable(device_name) 
                 << "' not found" << endl;
        return false;
#endif
    }

    // Try to open the device
    if (sane_open(device_name.toLatin1(), &m_saneHandle) != SANE_STATUS_GOOD) 
    {
        kDebug() << "openDevice: sane_open(\"" << qPrintable(device_name) 
                 << "\", &handle) failed!" << endl;
        return false;
    }
    //printf("openDevice: sane_open(\"%s\", &handle) == SANE_STATUS_GOOD\n", qPrintable(device_name));

    // Read the options (start with option 0 the number of parameters)
    num_option_d = sane_get_option_descriptor(m_saneHandle, 0);
    if (num_option_d == 0) 
    {
        return false;
    }
    QVarLengthArray<char> data(num_option_d->size);
    status = sane_control_option(m_saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) 
    {
        return false;
    }
    num_sane_options = *reinterpret_cast<SANE_Word*>(data.data());

    // read the rest of the options
    for (i=1; i<num_sane_options; i++) 
    {
        m_optList.append(new SaneOption(m_saneHandle, i));
    }

    // do the connections of the option parameters
    for (i=1; i<m_optList.size(); i++) 
    {
        connect (m_optList.at(i), SIGNAL(optsNeedReload()), 
                 this, SLOT(optReload()));

        connect (m_optList.at(i), SIGNAL(valsNeedReload()), 
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
    m_optArea = new QScrollArea(this);
    m_optArea->setWidgetResizable(true);
    m_optArea->setFrameShape(QFrame::NoFrame);
    opt_lay->addWidget(m_optArea, 0);
    opt_lay->setSpacing(2);
    opt_lay->setMargin(0);

    // Create the options interface
    createOptInterface();

    // create the preview
    m_preview = new PreviewArea(this);
    connect (m_preview, SIGNAL(newSelection(float,float,float,float)),
             this, SLOT(handleSelection(float,float,float,float)));
    m_prImg = m_preview->getImage();

    m_zInBtn  = new QPushButton();
    m_zInBtn->setIcon(SmallIcon("zoom-in"));
    m_zInBtn->setToolTip(i18n("Zoom in preview image"));
    m_zOutBtn = new QPushButton();
    m_zOutBtn->setIcon(SmallIcon("zoom-out"));
    m_zOutBtn->setToolTip(i18n("Zoom out preview image"));
    m_zSelBtn = new QPushButton();
    m_zSelBtn->setIcon(SmallIcon("file-find"));
    m_zSelBtn->setToolTip(i18n("Zoom to selection of preview image"));
    m_zFitBtn = new QPushButton();
    m_zFitBtn->setIcon(SmallIcon("zoom-best-fit"));
    m_zFitBtn->setToolTip(i18n("Zoom to fit preview image"));
    m_prevBtn  = new QPushButton();
    m_prevBtn->setIcon(SmallIcon("stamp"));
    m_prevBtn->setToolTip(i18n("Scan preview image from device"));
    m_scanBtn  = new QPushButton();
    m_scanBtn->setIcon(SmallIcon("scanner"));
    m_scanBtn->setToolTip(i18n("Scan final image from device"));

    connect(m_zInBtn, SIGNAL(clicked()), 
            m_preview, SLOT(zoomIn()));

    connect(m_zOutBtn, SIGNAL(clicked()), 
            m_preview, SLOT(zoomOut()));

    connect(m_zSelBtn, SIGNAL(clicked()), 
            m_preview, SLOT(zoomSel()));

    connect(m_zFitBtn, SIGNAL(clicked()), 
            m_preview, SLOT(zoom2Fit()));

    connect (m_scanBtn, SIGNAL(clicked()), 
             this, SLOT(scanFinal()));

    connect (m_prevBtn, SIGNAL(clicked()), 
             this, SLOT(scanPreview()));

    QHBoxLayout *zoom_layout = new QHBoxLayout;

    pr_layout->addWidget(m_preview, 100);
    pr_layout->addLayout(zoom_layout, 0);

    zoom_layout->addWidget(m_zInBtn);
    zoom_layout->addWidget(m_zOutBtn);
    zoom_layout->addWidget(m_zSelBtn);
    zoom_layout->addWidget(m_zFitBtn);
    zoom_layout->addStretch();
    zoom_layout->addWidget(m_prevBtn);
    zoom_layout->addWidget(m_scanBtn);

    //QHBoxLayout *scan_layout = new QHBoxLayout;
    //opt_lay->addLayout(scan_layout, 0);

    //scan_layout->addStretch();
    //scan_layout->addWidget(m_prevBtn);
    //scan_layout->addWidget(m_scanBtn);

    // try to set SaneWidget default values
    setDefaultValues();

    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();

    //m_preview->zoom2Fit();

    return true;
}

void SaneWidget::createOptInterface()
{
    // create the container widget
    QWidget *opt_container = new QWidget(m_optArea);
    m_optArea->setWidget(opt_container);
    QVBoxLayout *opt_layout = new QVBoxLayout(opt_container);
    opt_layout->setSpacing(4);
    opt_layout->setMargin(3);

    // add the options
    // (Should Vendor and model always be visible?)
    LabeledSeparator *model_label = new LabeledSeparator(opt_container, m_modelName);
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
        m_optMode = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // Bitdepth
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0) 
    {
        m_optDepth = option;
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
        m_optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    else if ((option = getOption(SANE_NAME_SCAN_X_RESOLUTION)) != 0) 
    {
        m_optRes = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_SCAN_Y_RESOLUTION)) != 0) 
    {
        m_optResY = option;
        option->createWidget(opt_container);
        opt_layout->addWidget(option->widget());
    }
    // scan area
    if ((option = getOption(SANE_NAME_SCAN_TL_X)) != 0) 
    {
        m_optTl = option;
        connect (option, SIGNAL(fValueRead(float)), 
                 this, SLOT(setTLX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_TL_Y)) != 0) 
    {
        m_optTlY = option;
        connect (option, SIGNAL(fValueRead(float)), 
                 this, SLOT(setTLY(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_X)) != 0) 
    {
        m_optBrX = option;
        connect (option, SIGNAL(fValueRead(float)), 
                 this, SLOT(setBRX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_Y)) != 0) 
    {
        m_optBrY = option;
        connect (option, SIGNAL(fValueRead(float)), 
                 this, SLOT(setBRY(float)));
    }

    // Color Options Frame
    m_colorOpts = new QWidget(opt_container);
    opt_layout->addWidget(m_colorOpts);
    QVBoxLayout *color_lay = new QVBoxLayout(m_colorOpts);
    color_lay->setSpacing(2);
    color_lay->setMargin(0);

    // add separator line
    color_lay->addSpacing(6);
    QFrame *line1 = new QFrame(m_colorOpts);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    color_lay->addWidget(line1);
    color_lay->addSpacing(2);

    if ((option = getOption(SANE_NAME_BRIGHTNESS)) != 0) 
    {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_CONTRAST)) != 0) 
    {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }

    // gamma tables
    QWidget *gamma_frm = new QWidget(m_colorOpts);
    color_lay->addWidget(gamma_frm);
    QVBoxLayout *gam_frm_l = new QVBoxLayout(gamma_frm);
    gam_frm_l->setSpacing(2);
    gam_frm_l->setMargin(0);

    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_R)) != 0) 
    {
        m_optGamR= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_G)) != 0) 
    {
        m_optGamG= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_B)) != 0) 
    {
        m_optGamB= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }

    if ((m_optGamR != 0) && (m_optGamG != 0) && (m_optGamB != 0)) 
    {
        LabeledGamma *lgamma = new LabeledGamma(m_colorOpts,
                                  QString(SANE_TITLE_GAMMA_VECTOR),
                                  m_optGamR->lgamma->size());
        color_lay->addWidget(lgamma);

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                m_optGamR->lgamma, SLOT(setValues(int,int,int)));

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                m_optGamG->lgamma, SLOT(setValues(int,int,int)));

        connect(lgamma, SIGNAL(gammaChanged(int,int,int)),
                m_optGamB->lgamma, SLOT(setValues(int,int,int)));

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
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_WHITE_LEVEL)) != 0) 
    {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }

    // Remaining (un known) Options Frame
    m_remainOpts = new QWidget(opt_container);
    opt_layout->addWidget(m_remainOpts);
    QVBoxLayout *remain_lay = new QVBoxLayout(m_remainOpts);
    remain_lay->setSpacing(2);
    remain_lay->setMargin(0);

    // add separator line
    remain_lay->addSpacing(4);
    QFrame *line2 = new QFrame(m_remainOpts);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    remain_lay->addWidget(line2);
    remain_lay->addSpacing(4);

    // add remaining parameters
    for (int i=0; i<m_optList.size(); i++) 
    {
        if ((m_optList.at(i)->widget() == 0) &&
             (m_optList.at(i)->name() != SANE_NAME_SCAN_TL_X) &&
             (m_optList.at(i)->name() != SANE_NAME_SCAN_TL_Y) &&
             (m_optList.at(i)->name() != SANE_NAME_SCAN_BR_X) &&
             (m_optList.at(i)->name() != SANE_NAME_SCAN_BR_Y) &&
             (m_optList.at(i)->sw_type() != SW_GROUP))
        {
            m_optList.at(i)->createWidget(m_remainOpts);
            remain_lay->addWidget(m_optList.at(i)->widget());
        }
    }

    // connect showing/hiding finctionality
    connect (opt_level, SIGNAL(activated(int)), 
             this, SLOT(opt_level_change(int)));

    // add a stretch to the end to ceep the parameters at the top
    opt_layout->addStretch();

    // encsure that you do not get a scrollbar at the bottom of the option of the options
    m_optArea->setMinimumWidth(opt_container->sizeHint().width()+20);

    // this could/should be set by saved settings.
    m_colorOpts->setVisible(false);
    m_remainOpts->setVisible(false);
}

void SaneWidget::opt_level_change(int level)
{
    if (m_colorOpts == 0) return;
    if (m_remainOpts == 0) return;

    switch (level)
    {
        case 1:
            m_colorOpts->setVisible(true);
            m_remainOpts->setVisible(false);
            break;
        case 2:
            m_colorOpts->setVisible(true);
            m_remainOpts->setVisible(true);
            break;
        default:
            m_colorOpts->setVisible(false);
            m_remainOpts->setVisible(false);
    }
}

void SaneWidget::setDefaultValues()
{
    SaneOption *option;

    // Try to get Color mode by default
    if ((option = getOption(SANE_NAME_SCAN_MODE)) != 0) 
    {
        option->setValue(QString(SANE_VALUE_SCAN_MODE_COLOR));
    }

    // Try to set 8 bit color
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0) 
    {
        option->setValue(8);
    }

    // Try to set Scan resolution to 600 DPI
    if (m_optRes != 0) 
    {
        m_optRes->setValue(600);
    }
}

void SaneWidget::scheduleValReload()
{
    m_rValTmr.start(5);
}

void SaneWidget::optReload()
{
    int i;
    //printf("Reload Options\n");

    for (i=0; i<m_optList.size(); i++)
    {
        m_optList.at(i)->readOption();
        // Also read the values
        m_optList.at(i)->readValue();
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

    for (i=0; i<m_optList.size(); i++)
    {
        m_optList.at(i)->readValue();
        /*
        if (m_optList.at(i)->getValue(&tmp)) {
            printf("option(%s)=%s\n",
                   qPrintable(m_optList.at(i)->name()),
                   qPrintable(tmp));
        }
        */
    }
}

SaneOption *SaneWidget::getOption(const QString &name)
{
    int i;

    for (i=0; i<m_optList.size(); i++)
    {
        if (m_optList.at(i)->name() == name) 
        {
            return m_optList.at(i);
        }
    }
    return 0;
}

void SaneWidget::handleSelection(float tl_x, float tl_y, float br_x, float br_y) {
    float max_x, max_y;

    //printf("handleSelection0: %f %f %f %f\n", tl_x, tl_y, br_x, br_y);
    if ((m_prImg->width()==0) || (m_prImg->height()==0)) return;

    m_optBrX->getMaxValue(&max_x);
    m_optBrY->getMaxValue(&max_y);
    float ftl_x = tl_x*max_x;
    float ftl_y = tl_y*max_y;
    float fbr_x = br_x*max_x;
    float fbr_y = br_y*max_y;

    //printf("handleSelection1: %f %f %f %f\n", ftl_x, ftl_y, fbr_x, fbr_y);

    if (m_optTl != 0) m_optTl->setValue(ftl_x);
    if (m_optTlY != 0) m_optTlY->setValue(ftl_y);
    if (m_optBrX != 0) m_optBrX->setValue(fbr_x);
    if (m_optBrY != 0) m_optBrY->setValue(fbr_y);
}

void SaneWidget::setTLX(float ftlx) 
{
    float max, ratio;

    //std::cout << "setTLX " << ftlx;
    m_optBrX->getMaxValue(&max);
    ratio = ftlx / max;
    //std::cout << " -> " << ratio << std::endl;
    m_preview->setTLX(ratio);
}

void SaneWidget::setTLY(float ftly) 
{
    float max, ratio;

    //std::cout << "setTLY " << ftly;
    m_optBrY->getMaxValue(&max);
    ratio = ftly / max;
    //std::cout << " -> " << ratio << std::endl;
    m_preview->setTLY(ratio);
}

void SaneWidget::setBRX(float fbrx) 
{
    float max, ratio;

    //std::cout << "setBRX " << fbrx;
    m_optBrX->getMaxValue(&max);
    ratio = fbrx / max;
    //std::cout << " -> " << ratio << std::endl;
    m_preview->setBRX(ratio);
}

void SaneWidget::setBRY(float fbry) 
{
    float max, ratio;

    //std::cout << "setBRY " << fbry;
    m_optBrY->getMaxValue(&max);
    ratio = fbry / max;
    //std::cout << " -> " << ratio << std::endl;
    m_preview->setBRY(ratio);
}

void SaneWidget::updatePreviewSize()
{
    SANE_Status status;
    int i, j, dpi;
    float max_x=0, max_y=0;

    // check if an update is necessary
    if (m_optBrX != 0) 
    {
        m_optBrX->getMaxValue(&max_x);
    }
    if (m_optBrY != 0) 
    {
        m_optBrY->getMaxValue(&max_y);
    }
    if ((max_x == m_previewWidth) && (max_y == m_previewHeight)) 
    {
        return;
    }

    m_previewWidth  = max_x;
    m_previewHeight = max_y;
    // set the scan area to the whole area
    if (m_optTl != 0) 
    {
        m_optTl->setValue(0);
    }
    if (m_optTlY != 0) 
    {
        m_optTlY->setValue(0);
    }

    if (m_optBrX != 0) 
    {
        m_optBrX->setValue(max_x);
    }
    if (m_optBrY != 0) 
    {
        m_optBrY->setValue(max_y);
    }

    // set the resopution to 100 dpi and increase if necessary
    dpi = 0;
    do 
    {
        // Increase the dpi value
        dpi += 100;
        if (m_optRes != 0) 
        {
            m_optRes->setValue(dpi);
        }
        if (m_optResY != 0) 
        {
            m_optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(m_saneHandle, &params);
        if (status != SANE_STATUS_GOOD) 
        {
            kDebug() << "ERROR: status="
                     << sane_strstatus(status) << endl;
            return;
        }
        //printf("dpi = %d\n", dpi);
        //printf("lines = %d\n", params.lines);
        //printf("pixels_per_line = %d\n", params.pixels_per_line);
        if (dpi > 800) break;
    } 
    while ((params.pixels_per_line < 300) || (params.lines < 300));


    if ((m_prImg->width() != params.pixels_per_line) || (m_prImg->height() != params.lines)) 
    {
        *m_prImg = QImage(params.pixels_per_line, params.lines, QImage::Format_RGB32);
        for (i=0; i<m_prImg->height(); i++) 
        {
            for (j=0; j<m_prImg->width(); j++) 
            {
                m_prImg->setPixel(j, i, qRgb(255,255,255));
            }
        }

        // clear the selection
        m_preview->clearSelection();

        // update the size of the preview widget.
        m_preview->updateScaledImg();
        //m_preview->resize(m_preview->sizeHint());
    }
}

void SaneWidget::scanPreview()
{
    SANE_Status status;
    float max;
    int i, j, dpi;

    //std::cout << "scanPreview" << std::endl;

    // store the current settings of parameters to be changed
    if (m_optDepth != 0) m_optDepth->storeCurrentData();
    if (m_optRes != 0) m_optRes->storeCurrentData();
    if (m_optResY != 0) m_optResY->storeCurrentData();
    if (m_optTl != 0) m_optTl->storeCurrentData();
    if (m_optTlY != 0) m_optTlY->storeCurrentData();
    if (m_optBrX != 0) m_optBrX->storeCurrentData();
    if (m_optBrY != 0) m_optBrY->storeCurrentData();

    // set 8 bits per color if possible
    if (m_optDepth != 0) 
    {
        m_optDepth->setValue(8);
    }

    // select the whole area
    if (m_optTl != 0) 
    {
        m_optTl->setValue(0);
    }
    if (m_optTlY != 0) 
    {
        m_optTlY->setValue(0);
    }

    if (m_optBrX != 0) 
    {
        m_optBrX->getMaxValue(&max);
        m_optBrX->setValue(max);
    }
    if (m_optBrY != 0) 
    {
        m_optBrY->getMaxValue(&max);
        m_optBrY->setValue(max);
    }

    // set the resopution to 100 dpi and increase if necessary
    dpi = 0;
    do 
    {
        // Increase the dpi value
        dpi += 100;
        if (m_optRes != 0) 
        {
            m_optRes->setValue(dpi);
        }
        if (m_optResY != 0) 
        {
            m_optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(m_saneHandle, &params);
        if (status != SANE_STATUS_GOOD) 
        {
            kDebug() << "ERROR: status="
                     << sane_strstatus(status) << endl;
            return;
        }
        //printf("dpi = %d\n", dpi);
        //printf("lines = %d\n", params.lines);
        //printf("pixels_per_line = %d\n", params.pixels_per_line);
        if (dpi > 800) break;
    } 
    while ((params.pixels_per_line < 300) || (params.lines < 300));

    // execute valReload if there is a pending value reload
    while (m_rValTmr.isActive()) 
    {
        m_rValTmr.stop();
        valReload();
    }

    // Start the scanning
    status = sane_start(m_saneHandle);
    if (status != SANE_STATUS_GOOD) 
    {
        kDebug() << "sane_start ERROR: status="
                 << sane_strstatus(status) << endl;

        sane_cancel(m_saneHandle);
        return;
    }

    status = sane_get_parameters(m_saneHandle, &params);
    if (status != SANE_STATUS_GOOD) 
    {
        kDebug() << "sane_get_parameters ERROR: status="
                 << sane_strstatus(status) << endl;
        sane_cancel(m_saneHandle);
        return;
    }

    //printf("format = %d\n", params.format);
    //printf("last_frame = %d\n", params.last_frame);
    //printf("lines = %d\n", params.lines);
    //printf("depth = %d\n", params.depth);
    //printf("pixels_per_line = %d\n", params.pixels_per_line);
    //printf("bytes_per_line = %d\n", params.bytes_per_line);

    // create a new image if necessary
    //(This image should be small so who cares about waisted memory :)
    // FIXME optimize size
    scan_img = m_prImg;
    if ((m_prImg->height() != params.lines) ||
         (m_prImg->width() != params.pixels_per_line))
    {
        *m_prImg = QImage(params.pixels_per_line, params.lines, QImage::Format_RGB32);
    }

    // clear the old image
    for (i=0; i<m_prImg->height(); i++) 
    {
        for (j=0; j<m_prImg->width(); j++) 
        {
            m_prImg->setPixel(j, i, qRgb(255,255,255));
        }
    }

    // update the size of the preview widget.
    m_preview->zoom2Fit();

    read_status = READ_ON_GOING;
    pixel_x = 0;
    pixel_y = 0;
    px_c_index = 0;

    setDisabled(true);

    while (read_status == READ_ON_GOING) 
    {
        processData();
    }

    m_preview->updateScaledImg();

    // restore the original settings of the changed parameters
    if (m_optDepth != 0) m_optDepth->restoreSavedData();
    if (m_optRes != 0) m_optRes->restoreSavedData();
    if (m_optResY != 0) m_optResY->restoreSavedData();
    if (m_optTl != 0) m_optTl->restoreSavedData();
    if (m_optTlY != 0) m_optTlY->restoreSavedData();
    if (m_optBrX != 0) m_optBrX->restoreSavedData();
    if (m_optBrY != 0) m_optBrY->restoreSavedData();

    setDisabled(false);
}

void SaneWidget::scanFinal()
{
    SANE_Status status;
    float v1,v2;

    //std::cout << "scanFinal" << std::endl;

    if ((m_optTl != 0) && (m_optBrX != 0)) 
    {
        m_optTl->getValue(&v1);
        m_optBrX->getValue(&v2);
        if (v1 == v2) 
        {
            m_optTl->setValue(0);
            m_optBrX->getMaxValue(&v2);
            m_optBrX->setValue(v2);
        }
    }

    if ((m_optTlY != 0) && (m_optBrY != 0)) 
    {
        m_optTlY->getValue(&v1);
        m_optBrY->getValue(&v2);
        if (v1 == v2) 
        {
            m_optTlY->setValue(0);
            m_optBrY->getMaxValue(&v2);
            m_optBrY->setValue(v2);
        }
    }

    // execute a pending value reload
    while (m_rValTmr.isActive()) 
    {
        m_rValTmr.stop();
        valReload();
    }

    // Start the scanning
    emit scanStart();
    status = sane_start(m_saneHandle);
    if (status != SANE_STATUS_GOOD) 
    {
        kDebug() << "sane_start ERROR: status="
                 << sane_strstatus(status) << endl;
        sane_cancel(m_saneHandle);
        return;
    }
    //printf("start OK\n");

    status = sane_get_parameters(m_saneHandle, &params);
    if (status != SANE_STATUS_GOOD) 
    {
        kDebug() << "sane_get_parameters ERROR: status=" 
                 << sane_strstatus(status) << endl;
        sane_cancel(m_saneHandle);
        return;
    }

    //printf("format = %d\n", params.format);
    //printf("last_frame = %d\n", params.last_frame);
    //printf("lines = %d\n", params.lines);
    //printf("depth = %d\n", params.depth);
    //printf("pixels_per_line = %d\n", params.pixels_per_line);
    //printf("bytes_per_line = %d\n", params.bytes_per_line);

    // create a new image
    //(This image should be small so who cares about waisted memory :)
    // FIXME optimize size
    scan_img  = &the_img;
    *scan_img = QImage(params.pixels_per_line, params.lines, QImage::Format_RGB32);

    // Signal for a progress dialog
    emit scanProgress(0);

    read_status = READ_ON_GOING;
    pixel_x = 0;
    pixel_y = 0;
    px_c_index = 0;

    setDisabled(true);
    while (read_status == READ_ON_GOING) 
    {
        processData();
    }
    if (read_status != READ_FINISHED) 
    {
        emit scanFaild();
    }
    setDisabled(false);
}

void SaneWidget::processData()
{
    SANE_Status status = SANE_STATUS_GOOD;
    SANE_Int read_bytes = 0;
    int i, j;

    //printf("Pre read()\n");
    status = sane_read(m_saneHandle, img_data, IMG_DATA_R_SIZE, &read_bytes);
    //printf("Post read() read=%d\n", read_bytes);

    if (status == SANE_STATUS_EOF) 
    {
        //printf("Read finished read_bytes=%d\n", read_bytes);
        if (pixel_y < params.lines) 
        {
            kDebug() << "pixel_y(" << pixel_y
                     << ") < params.lines(" << params.lines << ")" << endl;
            sleep(1);
            //sane_cancel(m_saneHandle);
        }
        if (params.last_frame == SANE_TRUE) 
        {
            // this is where it all ends well :)
            read_status = READ_FINISHED;
            if (scan_img == &the_img) 
            {
                emit scanDone();
                emit imageReady();
            }
            return;
        }
        else 
        {
            sane_start(m_saneHandle);
            if (status != SANE_STATUS_GOOD) 
            {
                kDebug() << "sane_start ERROR: status=" 
                         << sane_strstatus(status) << endl;
                sane_cancel(m_saneHandle);
                read_status = READ_ERROR;
                return;
            }
        }
    }
    else if (status != SANE_STATUS_GOOD) 
    {
        kDebug() << "Reading error, status=" 
                 << sane_strstatus(status) << endl;
        sane_cancel(m_saneHandle);
        read_status = READ_ERROR;
        return;
    }

    switch (params.format)
    {
        case SANE_FRAME_RGB:
            if (params.depth == 8) 
            {
                // go through the data
                for (i=0; i<read_bytes; i++) 
                {
                    if (pixel_y >= params.lines) 
                    {
                        kDebug() << "processData: reached image height before EOF" 
                                 << endl;
                        sane_cancel(m_saneHandle);
                        read_status = READ_ERROR;
                        return;
                    }
                    px_colors[px_c_index] = img_data[i];
                    px_c_index++;
                    if (px_c_index >= 3) px_c_index = 0;

                    if (px_c_index == 0) 
                    {
                        scan_img->setPixel(pixel_x, pixel_y,
                                         qRgb(px_colors[0],
                                              px_colors[1],
                                              px_colors[2]));
                        pixel_x++;
                        if (pixel_x >= params.pixels_per_line) 
                        {
                            pixel_x = 0;
                            pixel_y++;
                        }
                    }
                }
            }
            else 
            {
                kDebug() << "Only 8-bit colors are currently supported!" 
                         << endl;
                sane_cancel(m_saneHandle);
                read_status = READ_ERROR;
                return;
            }
            break;

        case SANE_FRAME_GRAY:
            if (params.depth == 8) 
            {
                for (i=0; i<read_bytes; i++) 
                {
                    if (pixel_y >= params.lines) 
                    {
                        kDebug() << "reached image height before EOF" << endl;
                        sane_cancel(m_saneHandle);
                        read_status = READ_ERROR;
                        return;
                    }
                    scan_img->setPixel(pixel_x, pixel_y,
                                       qRgb(img_data[i],
                                            img_data[i],
                                            img_data[i]));
                    pixel_x++;
                    if (pixel_x >= params.pixels_per_line) 
                    {
                        pixel_x = 0;
                        pixel_y++;
                    }
                }
            }
            else if (params.depth == 1)
            {
                for (i=0; i<read_bytes; i++) 
                {
                    if (pixel_y >= params.lines) 
                    {
                        kDebug() << "reached image height before EOF" << endl;
                        sane_cancel(m_saneHandle);
                        read_status = READ_ERROR;
                        return;
                    }
                    for (j=7; j>=0; j--) 
                    {
                        if ((img_data[i] & (1<<j)) == 0) 
                        {
                            scan_img->setPixel(pixel_x, pixel_y, qRgb(255,255,255));
                        }
                        else 
                        {
                            scan_img->setPixel(pixel_x, pixel_y, qRgb(0,0,0));
                        }
                        pixel_x++;
                        if(pixel_x >= params.pixels_per_line) 
                        {
                            pixel_x = 0;
                            pixel_y++;
                            break;
                        }
                        if (pixel_y >= params.lines) break;
                    }
                }
            }
            else 
            {
                kDebug() << "Only 1 and 8-bit colors are supported "
                            "for grayscale!" << endl;
                sane_cancel(m_saneHandle);
                read_status = READ_ERROR;
                return;
            }
            break;

        /*
        case SANE_FRAME_RED:
        case SANE_FRAME_GREEN:
        case SANE_FRAME_BLUE:
            for (i=0; i<read_bytes; i++) {
                printf("%d\n", pr_img_data[i]);
                if (pixel_y >= params.lines) {
                    printf("reached image height before EOF\n");
                    sane_cancel(m_saneHandle);
                    read_status = READ_ERROR;
                    return;
                }

                color = m_prImg->pixel(pixel_x, pixel_y);
                red   = qRed(color);
                green = qGreen(color);
                blue  = qBlue(color);

                switch(params.format)
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
                scan_img->setPixel(pixel_x, pixel_y, newColor);
                pixel_x++;
                if(pixel_x >= params.pixels_per_line) {
                    pixel_x = 0;
                    pixel_y++;
                    break;
                }
            }
            break;
        */
        default :
            kDebug() << "This frame format ( " << params.format
                     << ") is not yet suppoeted!" << endl;
            sane_cancel(m_saneHandle);
            read_status = READ_ERROR;
            return;
    }
    if (params.lines > 0) 
    {
        int new_progress = (int)( ((double)PROGRESS_MAX / params.lines) * pixel_y);
        if (abs (new_progress - progress) > 5) 
        {
            progress = new_progress;
            if (scan_img == m_prImg) 
            {
                m_preview->updateScaledImg();
            }
            if ((progress < PROGRESS_MAX) && (scan_img == &the_img)) 
            {
                emit scanProgress(progress);
            }
            qApp->processEvents();
        }
    }
}

QImage *SaneWidget::getFinalImage()
{
    return &the_img;
}

void SaneWidget::scanCancel()
{
    sane_cancel(m_saneHandle);
    read_status = READ_CANCEL;
}

bool SaneWidget::setIconColorMode(const QIcon &icon)
{
    if ((m_optMode != 0) && (m_optMode->widget() != 0)) 
    {
        m_optMode->setIconColorMode(icon);
        return true;
    }
    return false;
}

bool SaneWidget::setIconGrayMode(const QIcon &icon)
{
    if ((m_optMode != 0) && (m_optMode->widget() != 0)) 
    {
        m_optMode->setIconGrayMode(icon);
        return true;
    }
    return false;
}

bool SaneWidget::setIconBWMode(const QIcon &icon)
{
    if ((m_optMode != 0) && (m_optMode->widget() != 0)) 
    {
        m_optMode->setIconBWMode(icon);
        return true;
    }
    return false;
}

}  // NameSpace KSaneIface
