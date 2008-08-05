/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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

// Sane includes.
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

// Qt includes.
#include <QApplication>
#include <QVarLengthArray>
#include <QImage>
#include <QTimer>
#include <QScrollArea>
#include <QScrollBar>
#include <QList>
#include <QProgressBar>

// KDE includes
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KTabWidget>

// Local includes.
#include "sane_option.h"
#include "preview_area.h"
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


class KSaneWidgetPrivate
{
public:

    KSaneWidgetPrivate()
    {
        optsWidget    = 0;
        basic_options = 0;
        other_options = 0;
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
    KTabWidget         *optsWidget;
    QWidget            *basic_options;
    QWidget            *other_options;
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
    : QWidget(parent), d(new KSaneWidgetPrivate)
{
    SANE_Int    version;
    SANE_Status status;

    //kDebug() <<  "The language is:" << KGlobal::locale()->language();
    //kDebug() <<  "Languagelist" << KGlobal::locale()->languageList();
    KGlobal::locale()->insertCatalog("libksane");
    KGlobal::locale()->insertCatalog("sane-backends");

    status = sane_init(&version, 0);
    if (status != SANE_STATUS_GOOD) {
        kDebug() << "libksane: sane_init() failed("
                 << sane_strstatus(status) << ")";
    }
    else {
        //kDebug() << "Sane Version = "
        //         << SANE_VERSION_MAJOR(version) << "."
        //         << SANE_VERSION_MINOR(version) << "."
        //         << SANE_VERSION_BUILD(version);
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
    QStringList         dev_name_list;
    QString             tmp;
    SANE_Status         status;
    SANE_Device const **dev_list;

    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0) {
        //kDebug() << "i="       << i << " "
        //         << "name='"   << dev_list[i]->name   << "' "
        //         << "vendor='" << dev_list[i]->vendor << "' "
        //         << "model='"  << dev_list[i]->model  << "' "
        //         << "type='"   << dev_list[i]->type   << "'";
        tmp = QString(dev_list[i]->name);
        tmp += '\n' + QString(dev_list[i]->vendor);
        tmp += " : " + QString(dev_list[i]->model);
        dev_name_list += tmp;
        i++;
    }

    if (dev_name_list.isEmpty()) {
        KMessageBox::sorry(0, i18n("No scanner device has been found."));
        return QString();
    }

    if (dev_name_list.count() == 1) {
        // don't bother asking the user: we only have one choice!
        return dev_list[0]->name;
    }

    RadioSelect sel;
    sel.setWindowTitle(qApp->applicationName());
    i = sel.getSelectedIndex(parent, i18n("Select Scanner"), dev_name_list, 0);
    //kDebug() << "i=" << i;

    if ((i < 0) || (i >= dev_name_list.count())) {
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

    // don't bother trying to open if the device string is empty
    if (device_name.isEmpty()) {
        return false;
    }
    // get the device list to get the vendor and model info
    status = sane_get_devices(&dev_list, SANE_TRUE);

    while(dev_list[i] != 0) {
        if (QString(dev_list[i]->name) == device_name) {
            d->modelName = QString(dev_list[i]->vendor) + ' ' + QString(dev_list[i]->model);
            d->vendor    = QString(dev_list[i]->vendor);
            d->model     = QString(dev_list[i]->model);
            break;
        }
        i++;
    }

    if (device_name == "test") {
        d->modelName = "Test Scanner";
        d->vendor    = "Test";
        d->model     = "Scanner";
    }

    // Try to open the device
    if (sane_open(device_name.toLatin1(), &d->saneHandle) != SANE_STATUS_GOOD) {
        //kDebug() << "sane_open(\"" << device_name << "\", &handle) failed!";
        return false;
    }

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

    // Create Options Widget
    d->optsWidget = new KTabWidget;
     // Create the options interface
    createOptInterface();

    opt_lay->addWidget(d->optsWidget, 0);
    opt_lay->setSpacing(2);
    opt_lay->setMargin(0);

    // create the preview
    d->previewArea = new PreviewArea(this);
    connect(d->previewArea, SIGNAL(newSelection(float, float, float, float)),
            this, SLOT(handleSelection(float, float, float, float)));
    d->previewImg = d->previewArea->getImage();

    d->zInBtn  = new QPushButton(this);
    d->zInBtn->setIcon(KIcon("zoom-in"));
    d->zInBtn->setToolTip(i18n("Zoom In"));
    d->zOutBtn = new QPushButton(this);
    d->zOutBtn->setIcon(KIcon("zoom-out"));
    d->zOutBtn->setToolTip(i18n("Zoom Out"));
    d->zSelBtn = new QPushButton(this);
    d->zSelBtn->setIcon(KIcon("zoom-fit-best"));
    d->zSelBtn->setToolTip(i18n("Zoom to Selection"));
    d->zFitBtn = new QPushButton(this);
    d->zFitBtn->setIcon(KIcon("document-preview"));
    d->zFitBtn->setToolTip(i18n("Zoom to Fit"));

    d->progressBar = new QProgressBar(this);
    d->progressBar->hide();
    d->cancelBtn   = new QPushButton(this);
    d->cancelBtn->setIcon(KIcon("process-stop"));
    d->cancelBtn->setToolTip(i18n("Cancel current scan operation"));
    d->cancelBtn->hide();

    d->prevBtn = new QPushButton(this);
    d->prevBtn->setIcon(KIcon("document-import"));
    d->prevBtn->setToolTip(i18n("Scan Preview Image"));
    d->scanBtn = new QPushButton(this);
    d->scanBtn->setIcon(KIcon("document-save"));
    d->scanBtn->setToolTip(i18n("Scan Final Image"));

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
    QHBoxLayout *progress_lay = new QHBoxLayout;

    pr_layout->addWidget(d->previewArea, 100);
    pr_layout->addLayout(progress_lay, 0);
    pr_layout->addLayout(zoom_layout, 0);

    progress_lay->addWidget(d->progressBar, 100);
    progress_lay->addWidget(d->cancelBtn, 0);

    zoom_layout->addWidget(d->zInBtn);
    zoom_layout->addWidget(d->zOutBtn);
    zoom_layout->addWidget(d->zSelBtn);
    zoom_layout->addWidget(d->zFitBtn);
    zoom_layout->addStretch(100);
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
    // remove all tabs
    d->optsWidget->clear();

    // Basic options
    QScrollArea *basic_area = new QScrollArea(d->optsWidget);
    basic_area->setWidgetResizable(true);
    basic_area->setFrameShape(QFrame::NoFrame);
    d->optsWidget->addTab(basic_area, i18n("Basic Options"));

    d->basic_options = new QWidget(basic_area);
    basic_area->setWidget(d->basic_options);

    QVBoxLayout *basic_layout = new QVBoxLayout(d->basic_options);
    basic_layout->setSpacing(4);
    basic_layout->setMargin(3);


    SaneOption *option;
    // Scan Source
    if ((option = d->getOption(SANE_NAME_SCAN_SOURCE)) != 0) {
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // film-type (note: No translation)
    if ((option = d->getOption(QString("film-type"))) != 0) {
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    else if ((option = d->getOption(SANE_NAME_NEGATIVE)) != 0) {
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // Scan mode
    if ((option = d->getOption(SANE_NAME_SCAN_MODE)) != 0) {
        d->optMode = option;
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // Bitdepth
    if ((option = d->getOption(SANE_NAME_BIT_DEPTH)) != 0) {
        d->optDepth = option;
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // Threshold
    if ((option = d->getOption(SANE_NAME_THRESHOLD)) != 0) {
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // Resolution
    if ((option = d->getOption(SANE_NAME_SCAN_RESOLUTION)) != 0) {
        d->optRes = option;
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    else if ((option = d->getOption(SANE_NAME_SCAN_X_RESOLUTION)) != 0) {
        d->optRes = option;
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    if ((option = d->getOption(SANE_NAME_SCAN_Y_RESOLUTION)) != 0) {
        d->optResY = option;
        option->createWidget(d->basic_options);
        basic_layout->addWidget(option->widget());
    }
    // scan area (Do not add the widgets)
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
    d->colorOpts = new QWidget(d->basic_options);
    basic_layout->addWidget(d->colorOpts);
    QVBoxLayout *color_lay = new QVBoxLayout(d->colorOpts);
    color_lay->setSpacing(2);
    color_lay->setMargin(0);

    // Color correction
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
        LabeledGamma *gamma = reinterpret_cast<LabeledGamma *>(d->optGamR->widget());
        LabeledGamma *one_gamma = new LabeledGamma(d->colorOpts,
                                  i18n(SANE_TITLE_GAMMA_VECTOR),
                                       gamma->size());

        color_lay->addWidget(one_gamma);

        one_gamma->setToolTip(i18n(SANE_DESC_GAMMA_VECTOR));

        connect(one_gamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamR->widget(), SLOT(setValues(int,int,int)));

        connect(one_gamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamG->widget(), SLOT(setValues(int,int,int)));

        connect(one_gamma, SIGNAL(gammaChanged(int,int,int)),
                d->optGamB->widget(), SLOT(setValues(int,int,int)));

        QCheckBox *split_gam_btn = new QCheckBox(i18n("Separate color intensity tables"),
                                                 d->basic_options);
        color_lay->addWidget(split_gam_btn);

        connect (split_gam_btn, SIGNAL(toggled(bool)),
                 gamma_frm, SLOT(setVisible(bool)));

        connect (split_gam_btn, SIGNAL(toggled(bool)),
                 one_gamma, SLOT(setHidden(bool)));

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

    // add a stretch to the end to keep the parameters at the top
    basic_layout->addStretch();

    // calculeate sizes
    /*
    int lab_w=0, rest_w=0;
    int lab_tmp, rest_tmp;
    for (int i=0; i<d->optList.size(); i++) {
        if (d->optList.at(i)->widget() != 0) {
            d->optList.at(i)->widgetSizeHints(&lab_tmp, &rest_tmp);
            lab_tmp = qMax(lab_tmp, lab_w);
            rest_w  = qMax(rest_w, rest_tmp);
        }
    }
    for (int i=0; i<d->optList.size(); i++) {
        if (d->optList.at(i)->widget() != 0) {
            d->optList.at(i)->setColumnWidths(lab_w, rest_w);
        }
    }
    */

    // Remaining (un known) Options tabs
    QScrollArea *other_area = new QScrollArea(d->optsWidget);
    other_area->setWidgetResizable(true);
    other_area->setFrameShape(QFrame::NoFrame);
    d->optsWidget->addTab(other_area, i18n("Other Options"));

    d->other_options = new QWidget(other_area);
    other_area->setWidget(d->other_options);

    QVBoxLayout *other_layout = new QVBoxLayout(d->other_options);
    other_layout->setSpacing(2);
    other_layout->setMargin(0);

    // add remaining parameters
    for (int i=0; i<d->optList.size(); i++) {
        if ((d->optList.at(i)->widget() == 0) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_TL_X) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_TL_Y) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_BR_X) &&
             (d->optList.at(i)->name() != SANE_NAME_SCAN_BR_Y) &&
             (d->optList.at(i)->sw_type() != SW_GROUP))
        {
            d->optList.at(i)->createWidget(d->other_options);
            other_layout->addWidget(d->optList.at(i)->widget());
        }
    }

    // calculeate sizes
    /*
    int lab_w=0, rest_w=0;
    int lab_tmp, rest_tmp;
    for (int i=0; i<d->optList.size(); i++) {
        if (d->optList.at(i)->widget() != 0) {
            d->optList.at(i)->widgetSizeHints(&lab_tmp, &rest_tmp);
            lab_w = qMax(lab_tmp, lab_w);
            rest_w  = qMax(rest_w, rest_tmp);
        }
    }
    for (int i=0; i<d->optList.size(); i++) {
        if (d->optList.at(i)->widget() != 0) {
            d->optList.at(i)->setColumnWidths(lab_w, rest_w);
        }
    }
    */
    // add a stretch to the end to keep the parameters at the top
    other_layout->addStretch();

    // encsure that we do not get a scrollbar at the bottom of the option of the options
    int min_width = d->basic_options->sizeHint().width();
    if (min_width < d->other_options->sizeHint().width()) {
        min_width = d->other_options->sizeHint().width();
    }

    d->optsWidget->setMinimumWidth(min_width +
            basic_area->verticalScrollBar()->sizeHint().width() + 5);

}


void KSaneWidget::setDefaultValues()
{
    SaneOption *option;

    // Try to get Color mode by default
    if ((option = d->getOption(SANE_NAME_SCAN_MODE)) != 0) {
        option->setValue(i18n(SANE_VALUE_SCAN_MODE_COLOR));
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

    for (i=0; i<d->optList.size(); i++) {
        d->optList.at(i)->readOption();
        // Also read the values
        d->optList.at(i)->readValue();
    }
    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();

    // encsure that we do not get a scrollbar at the bottom of the option of the options
    int min_width = d->basic_options->sizeHint().width();
    if (min_width < d->other_options->sizeHint().width()) {
        min_width = d->other_options->sizeHint().width();
    }

    d->optsWidget->setMinimumWidth(min_width + 32);

}

void KSaneWidget::valReload()
{
    int i;
    QString tmp;

    for (i=0; i<d->optList.size(); i++) {
        d->optList.at(i)->readValue();
    }
}

void KSaneWidget::handleSelection(float tl_x, float tl_y, float br_x, float br_y) {
    float max_x, max_y;

    if ((d->previewImg->width()==0) || (d->previewImg->height()==0)) return;

    d->optBrX->getMaxValue(&max_x);
    d->optBrY->getMaxValue(&max_y);
    float ftl_x = tl_x*max_x;
    float ftl_y = tl_y*max_y;
    float fbr_x = br_x*max_x;
    float fbr_y = br_y*max_y;

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
        if ((status == SANE_STATUS_NO_DOCS) ||
             (status == SANE_STATUS_JAMMED) ||
             (status == SANE_STATUS_COVER_OPEN) ||
             (status == SANE_STATUS_DEVICE_BUSY) ||
             (status == SANE_STATUS_ACCESS_DENIED))
        {
            KMessageBox::sorry(0, i18n(sane_strstatus(status)));
        }
        else {
            kDebug() << "sane_start=" << sane_strstatus(status);
        }
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
        if ((status == SANE_STATUS_NO_DOCS) ||
             (status == SANE_STATUS_JAMMED) ||
             (status == SANE_STATUS_COVER_OPEN) ||
             (status == SANE_STATUS_DEVICE_BUSY) ||
             (status == SANE_STATUS_ACCESS_DENIED))
        {
            KMessageBox::sorry(0, i18n(sane_strstatus(status)));
        }
        else {
            kDebug() << "sane_start=" << sane_strstatus(status);
        }
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

    status = sane_read(d->saneHandle, d->saneReadBuffer, IMG_DATA_R_SIZE, &read_bytes);

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
                sane_cancel(d->saneHandle);
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

    // copy the data to the buffer
    if (d->isPreview) {
        copyToPreview((int)read_bytes);
    }
    else {
        copyToScanData((int)read_bytes);
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



QImage KSaneWidget::toQImage(const QByteArray &data,
                              int width,
                              int height,
                              int bytes_per_line,
                              ImageFormat format)
{
    QImage img;
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
            return img;

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
            return img;

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
            KMessageBox::sorry(0, i18n("The image data contained 16 bits per color, "
                    "but the color depth has been truncated to 8 bits per color."));
            return img;

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
            return img;

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
            KMessageBox::sorry(0, i18n("The image data contained 16 bits per color, "
                    "but the color depth has been truncated to 8 bits per color."));
            return img;

        case FormatNone:
            break;
    }
    kDebug() << "Unsupported conversion";
    return img;
}

bool KSaneWidget::makeQImage(const QByteArray &data,
                             int width,
                             int height,
                             int bytes_per_line,
                             ImageFormat format,
                             QImage &img)
{
    img = toQImage(data, width, height, bytes_per_line, format);
    if (img.isNull()) return false;
    return true;
}

void KSaneWidget::scanCancel()
{
    sane_cancel(d->saneHandle);
    d->readStatus = READ_CANCEL;
}


void KSaneWidget::setBusy(bool busy)
{
    d->optsWidget->setDisabled(busy);
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
