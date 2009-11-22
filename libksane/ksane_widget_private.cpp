/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes.
#include "ksane_widget_private.h"
#include "ksane_widget_private.moc"

// Qt includes.
#include <QImage>
#include <QScrollArea>
#include <QScrollBar>
#include <QList>
#include <QLabel>

// KDE includes
#include <KPushButton>

#define SCALED_PREVIEW_MAX_SIDE 400

static const int ActiveSelection = 100000;

namespace KSaneIface
{

KSaneWidgetPrivate::KSaneWidgetPrivate()
{
    // Device independent UI variables
    m_optsTabWidget = 0;
    m_basicOptsTab  = 0;
    m_otherOptsTab  = 0;
    m_colorOpts     = 0;
    m_scanBtn       = 0;
    m_prevBtn       = 0;
    m_zInBtn        = 0;
    m_zOutBtn       = 0;
    m_zSelBtn       = 0;
    m_zFitBtn       = 0;
    m_cancelBtn     = 0;
    m_previewViewer = 0;
    m_autoSelect    = true;
    m_selIndex      = ActiveSelection;
    m_warmingUp     = 0;
    m_progressBar   = 0;

    // scanning variables
    m_pixel_x       = 0;
    m_pixel_y       = 0;
    m_px_c_index    = 0;
    m_frame_t_count = 0;
    m_frameSize     = 0;
    m_frameRead     = 0;
    m_dataSize      = 0;
    m_readStatus    = READ_READY;
    m_isPreview     = false;
    m_timeSinceUpd.restart();
    m_saneHandle    = 0;
    m_readThread    = 0;
    m_splitGamChB   = 0;
    m_commonGamma   = 0;
    
    clearDeviceOptions();
}

void KSaneWidgetPrivate::clearDeviceOptions()
{
    m_optSource     = 0;
    m_optMode       = 0;
    m_optDepth      = 0;
    m_optRes        = 0;
    m_optResY       = 0;
    m_optTlX        = 0;
    m_optTlY        = 0;
    m_optBrX        = 0;
    m_optBrY        = 0;
    m_optGamR       = 0;
    m_optGamG       = 0;
    m_optGamB       = 0;
    m_optPreview    = 0;
    
    // delete all the options in the list.
    while (!m_optList.isEmpty()) {
        delete m_optList.takeFirst();
    }

    // remove the remaining layouts/widgets and read thread
    delete m_commonGamma;
    m_commonGamma = 0;

    delete m_splitGamChB;
    m_splitGamChB = 0;

    delete m_basicOptsTab;
    m_basicOptsTab = 0;

    delete m_otherOptsTab;
    m_otherOptsTab = 0;

    delete m_readThread;
    m_readThread = 0;
}

KSaneWidget::ImageFormat KSaneWidgetPrivate::getImgFormat(SANE_Parameters &params)
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

int KSaneWidgetPrivate::getBytesPerLines(SANE_Parameters &params)
{
    switch (getImgFormat(params))
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


KSaneOption *KSaneWidgetPrivate::getOption(const QString &name)
{
    int i;
    for (i=0; i<m_optList.size(); i++) {
        if (m_optList.at(i)->name() == name) {
            return m_optList.at(i);
        }
    }
    return 0;
}

void KSaneWidgetPrivate::createOptInterface()
{
    m_basicOptsTab = new QWidget;
    m_basicScrollA->setWidget(m_basicOptsTab);

    QVBoxLayout *basic_layout = new QVBoxLayout(m_basicOptsTab);
    basic_layout->setSpacing(4);
    basic_layout->setMargin(3);

    KSaneOption *option;
    // Scan Source
    if ((option = getOption(SANE_NAME_SCAN_SOURCE)) != 0) {
        m_optSource = option;
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    // film-type (note: No translation)
    if ((option = getOption(QString("film-type"))) != 0) {
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    else if ((option = getOption(SANE_NAME_NEGATIVE)) != 0) {
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    // Scan mode
    if ((option = getOption(SANE_NAME_SCAN_MODE)) != 0) {
        m_optMode = option;
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    // Bitdepth
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0) {
        m_optDepth = option;
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    // Threshold
    if ((option = getOption(SANE_NAME_THRESHOLD)) != 0) {
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    // Resolution
    if ((option = getOption(SANE_NAME_SCAN_RESOLUTION)) != 0) {
        m_optRes = option;
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
    }
    else {
        // TODO: add a combined x and y resolution widget.
        if ((option = getOption(SANE_NAME_SCAN_X_RESOLUTION)) != 0) {
        m_optRes = option;
        option->createWidget(m_basicOptsTab);
        basic_layout->addWidget(option->widget());
        }
        if ((option = getOption(SANE_NAME_SCAN_Y_RESOLUTION)) != 0) {
            m_optResY = option;
            option->createWidget(m_basicOptsTab);
            basic_layout->addWidget(option->widget());
        }
    }
    // scan area (Do not add the widgets)
    if ((option = getOption(SANE_NAME_SCAN_TL_X)) != 0) {
        m_optTlX = option;
        connect (option, SIGNAL(fValueRead(float)), this, SLOT(setTLX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_TL_Y)) != 0) {
        m_optTlY = option;
        connect (option, SIGNAL(fValueRead(float)), this, SLOT(setTLY(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_X)) != 0) {
        m_optBrX = option;
        connect (option, SIGNAL(fValueRead(float)), this, SLOT(setBRX(float)));
    }
    if ((option = getOption(SANE_NAME_SCAN_BR_Y)) != 0) {
        m_optBrY = option;
        connect (option, SIGNAL(fValueRead(float)), this, SLOT(setBRY(float)));
    }

    // Color Options Frame
    m_colorOpts = new QWidget;
    basic_layout->addWidget(m_colorOpts);
    QVBoxLayout *color_lay = new QVBoxLayout(m_colorOpts);
    color_lay->setSpacing(4);
    color_lay->setMargin(3);

    // Add Color correction to the color "frame"
    if ((option = getOption(SANE_NAME_BRIGHTNESS)) != 0) {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_CONTRAST)) != 0) {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    
    // Add gamma tables to the color "frame"
    QWidget *gamma_frm = new QWidget;
    color_lay->addWidget(gamma_frm);
    QVBoxLayout *gam_frm_l = new QVBoxLayout(gamma_frm);
    gam_frm_l->setSpacing(4);
    gam_frm_l->setMargin(3);
    
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_R)) != 0) {
        m_optGamR= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_G)) != 0) {
        m_optGamG= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_GAMMA_VECTOR_B)) != 0) {
        m_optGamB= option;
        option->createWidget(gamma_frm);
        gam_frm_l->addWidget(option->widget());
    }
    
    
    if ((m_optGamR != 0) && (m_optGamG != 0) && (m_optGamB != 0)) {
        LabeledGamma *gamma = reinterpret_cast<LabeledGamma *>(m_optGamR->widget());
        m_commonGamma = new LabeledGamma(m_colorOpts, i18n(SANE_TITLE_GAMMA_VECTOR), gamma->size());
        
        color_lay->addWidget(m_commonGamma);
        
        m_commonGamma->setToolTip(i18n(SANE_DESC_GAMMA_VECTOR));
        
        connect(m_commonGamma, SIGNAL(gammaChanged(int,int,int)), m_optGamR->widget(), SLOT(setValues(int,int,int)));
        connect(m_commonGamma, SIGNAL(gammaChanged(int,int,int)), m_optGamG->widget(), SLOT(setValues(int,int,int)));
        connect(m_commonGamma, SIGNAL(gammaChanged(int,int,int)), m_optGamB->widget(), SLOT(setValues(int,int,int)));

        m_splitGamChB = new QCheckBox(i18n("Separate color intensity tables"),
                                                  m_basicOptsTab);
                                                  color_lay->addWidget(m_splitGamChB);
        
        connect (m_splitGamChB, SIGNAL(toggled(bool)), gamma_frm, SLOT(setVisible(bool)));
        connect (m_splitGamChB, SIGNAL(toggled(bool)), m_commonGamma, SLOT(setHidden(bool)));
        
        gamma_frm->hide();
    }
    
    if ((option = getOption(SANE_NAME_BLACK_LEVEL)) != 0) {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    if ((option = getOption(SANE_NAME_WHITE_LEVEL)) != 0) {
        option->createWidget(m_colorOpts);
        color_lay->addWidget(option->widget());
    }
    
    // add a stretch to the end to keep the parameters at the top
    basic_layout->addStretch();

    
    // Remaining (un known) options go to the "Other Options"
    m_otherOptsTab = new QWidget;
    m_otherScrollA->setWidget(m_otherOptsTab);
    
    QVBoxLayout *other_layout = new QVBoxLayout(m_otherOptsTab);
    other_layout->setSpacing(4);
    other_layout->setMargin(3);
    
    // add the remaining parameters
    for (int i=0; i<m_optList.size(); i++) {
        if ((m_optList.at(i)->widget() == 0) &&
            (m_optList.at(i)->name() != SANE_NAME_SCAN_TL_X) &&
            (m_optList.at(i)->name() != SANE_NAME_SCAN_TL_Y) &&
            (m_optList.at(i)->name() != SANE_NAME_SCAN_BR_X) &&
            (m_optList.at(i)->name() != SANE_NAME_SCAN_BR_Y) &&
            (m_optList.at(i)->hasGui()))
        {
            m_optList.at(i)->createWidget(m_otherOptsTab);
            other_layout->addWidget(m_optList.at(i)->widget());
        }
    }
    
    // save a pointer to the preview option if possible
    if ((option = getOption(SANE_NAME_PREVIEW)) != 0) {
        m_optPreview = option;
    }
    
    // add a stretch to the end to keep the parameters at the top
    other_layout->addStretch();
    
    // encsure that we do not get a scrollbar at the bottom of the option of the options
    int min_width = m_basicOptsTab->sizeHint().width();
    if (min_width < m_otherOptsTab->sizeHint().width()) {
        min_width = m_otherOptsTab->sizeHint().width();
    }
    
    m_optsTabWidget->setMinimumWidth(min_width +
    m_basicScrollA->verticalScrollBar()->sizeHint().width() + 5);
}


void KSaneWidgetPrivate::setDefaultValues()
{
    KSaneOption *option;
    
    // Try to get Color mode by default
    if ((option = getOption(SANE_NAME_SCAN_MODE)) != 0) {
        option->setValue(i18n(SANE_VALUE_SCAN_MODE_COLOR));
    }
    
    // Try to set 8 bit color
    if ((option = getOption(SANE_NAME_BIT_DEPTH)) != 0) {
        option->setValue(8);
    }
    
    // Try to set Scan resolution to 600 DPI
    if (m_optRes != 0) {
        m_optRes->setValue(600);
    }
}

void KSaneWidgetPrivate::scheduleValReload()
{
    m_readValsTmr.start(5);
}

void KSaneWidgetPrivate::optReload()
{
    int i;
    
    for (i=0; i<m_optList.size(); i++) {
        m_optList.at(i)->readOption();
        // Also read the values
        m_optList.at(i)->readValue();
    }
    // estimate the preview size and create an empty image
    // this is done so that you can select scanarea without
    // having to scan a preview.
    updatePreviewSize();
    
    // encsure that we do not get a scrollbar at the bottom of the option of the options
    int min_width = m_basicOptsTab->sizeHint().width();
    if (min_width < m_otherOptsTab->sizeHint().width()) {
        min_width = m_otherOptsTab->sizeHint().width();
    }
    
    m_optsTabWidget->setMinimumWidth(min_width + 32);
    
}

void KSaneWidgetPrivate::valReload()
{
    int i;
    QString tmp;
    
    for (i=0; i<m_optList.size(); i++) {
        m_optList.at(i)->readValue();
    }
}

void KSaneWidgetPrivate::handleSelection(float tl_x, float tl_y, float br_x, float br_y) {
    
    if ((m_optTlX == 0) || (m_optTlY == 0) || (m_optBrX == 0) || (m_optBrY == 0)) {
        // clear the selection since we can not set one
        m_previewViewer->setTLX(0);
        m_previewViewer->setTLY(0);
        m_previewViewer->setBRX(0);
        m_previewViewer->setBRY(0);
        return;
    }
    float max_x, max_y;
    
    if ((m_previewImg.width()==0) || (m_previewImg.height()==0)) return;
    
    m_optBrX->getMaxValue(max_x);
    m_optBrY->getMaxValue(max_y);
    float ftl_x = tl_x*max_x;
    float ftl_y = tl_y*max_y;
    float fbr_x = br_x*max_x;
    float fbr_y = br_y*max_y;
    
    m_optTlX->setValue(ftl_x);
    m_optTlY->setValue(ftl_y);
    m_optBrX->setValue(fbr_x);
    m_optBrY->setValue(fbr_y);
}

void KSaneWidgetPrivate::setTLX(float ftlx)
{
    // ignore this during an active scan
    if (m_readStatus == READ_ON_GOING) return;
    
    float max, ratio;
    
    //kDebug(51004) << "setTLX " << ftlx;
    m_optBrX->getMaxValue(max);
    ratio = ftlx / max;
    //kDebug(51004) << " -> " << ratio;
    m_previewViewer->setTLX(ratio);
}

void KSaneWidgetPrivate::setTLY(float ftly)
{
    // ignore this during an active scan
    if (m_readStatus == READ_ON_GOING) return;
    
    float max, ratio;
    
    //kDebug(51004) << "setTLY " << ftly;
    m_optBrY->getMaxValue(max);
    ratio = ftly / max;
    //kDebug(51004) << " -> " << ratio;
    m_previewViewer->setTLY(ratio);
}

void KSaneWidgetPrivate::setBRX(float fbrx)
{
    // ignore this during an active scan
    if (m_readStatus == READ_ON_GOING) return;
    
    float max, ratio;
    
    //kDebug(51004) << "setBRX " << fbrx;
    m_optBrX->getMaxValue(max);
    ratio = fbrx / max;
    //kDebug(51004) << " -> " << ratio;
    m_previewViewer->setBRX(ratio);
}

void KSaneWidgetPrivate::setBRY(float fbry)
{
    // ignore this during an active scan
    if (m_readStatus == READ_ON_GOING) return;
    
    float max, ratio;
    
    //kDebug(51004) << "setBRY " << fbry;
    m_optBrY->getMaxValue(max);
    ratio = fbry / max;
    //kDebug(51004) << " -> " << ratio;
    m_previewViewer->setBRY(ratio);
}

void KSaneWidgetPrivate::updatePreviewSize()
{
    float max_x=0, max_y=0;
    float ratio;
    int x,y;
    
    // check if an update is necessary
    if (m_optBrX != 0) {
        m_optBrX->getMaxValue(max_x);
    }
    if (m_optBrY != 0) {
        m_optBrY->getMaxValue(max_y);
    }
    if ((max_x == m_previewWidth) && (max_y == m_previewHeight)) {
        //kDebug(51004) << "no preview width";
        return;
    }
    
    m_previewWidth  = max_x;
    m_previewHeight = max_y;
    // set the scan area to the whole area
    if (m_optTlX != 0) {
        m_optTlX->setValue(0);
    }
    if (m_optTlY != 0) {
        m_optTlY->setValue(0);
    }
    
    if (m_optBrX != 0) {
        m_optBrX->setValue(max_x);
    }
    if (m_optBrY != 0) {
        m_optBrY->setValue(max_y);
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
    
    m_previewImg = QImage(x, y, QImage::Format_RGB32);
    m_previewImg.fill(0xFFFFFFFF);
    
    // set the new image
    m_previewViewer->setQImage(&m_previewImg);
}

void KSaneWidgetPrivate::scanPreview()
{
    SANE_Status status;
    float max_x, max_y;
    float dpi;
    
    if (m_readStatus == READ_ON_GOING) return;
    
    m_readStatus  = READ_ON_GOING;
    m_isPreview = true;
    
    // store the current settings of parameters to be changed
    if (m_optDepth != 0) m_optDepth->storeCurrentData();
    if (m_optRes != 0) m_optRes->storeCurrentData();
    if (m_optResY != 0) m_optResY->storeCurrentData();
    if (m_optTlX != 0) m_optTlX->storeCurrentData();
    if (m_optTlY != 0) m_optTlY->storeCurrentData();
    if (m_optBrX != 0) m_optBrX->storeCurrentData();
    if (m_optBrY != 0) m_optBrY->storeCurrentData();
    if (m_optPreview != 0) m_optPreview->storeCurrentData();
    
    // check if we can modify the selection
    if ((m_optTlX != 0) && (m_optTlY != 0) &&
        (m_optBrX != 0) && (m_optBrY != 0))
    {
        // get maximums
        m_optBrX->getMaxValue(max_x);
        m_optBrY->getMaxValue(max_y);
        // select the whole area
        m_optTlX->setValue(0);
        m_optTlY->setValue(0);
        m_optBrX->setValue(max_x);
        m_optBrY->setValue(max_y);
        
    }
    else {
        // no use to try auto selections if you can not use them
        m_autoSelect = false;
    }
    
    // set the resopution to 100 dpi and increase if necessary
    m_optRes->getMinValue(dpi);
    do {
        if (m_optRes != 0) {
            m_optRes->setValue(dpi);
        }
        if ((m_optResY != 0) && (m_optRes->name() == SANE_NAME_SCAN_X_RESOLUTION)) {
            m_optResY->setValue(dpi);
        }
        //check what image size we would get in a scan
        status = sane_get_parameters(m_saneHandle, &m_params);
        if (status != SANE_STATUS_GOOD) {
            kDebug(51004) << "sane_get_parameters=" << sane_strstatus(status);
            scanDone();
            return;
        }
        if (dpi > 300) break;
        // Increase the dpi value
        dpi += 25.0;
    }
    while ((m_params.pixels_per_line < 300) || (m_params.lines < 300));
    
    // set preview option to true if possible
    if (m_optPreview != 0) m_optPreview->setValue(1);
    
    // execute valReload if there is a pending value reload
    while (m_readValsTmr.isActive()) {
        m_readValsTmr.stop();
        valReload();
    }
    
    setBusy(true);
    
    //We could use:QMetaObject::invokeMethod(this, "startScan", Qt::QueuedConnection);
    // but the timer is still needed so, we stick to that :)
    m_startScanTmr.start(0);
}

void KSaneWidgetPrivate::scanFinal()
{
    if (m_readStatus == READ_ON_GOING) return;
    
    float x1=0,y1=0, x2=0,y2=0, max_x, max_y;
    
    m_isPreview = false;

    if ((m_readStatus == READ_READY) || (m_readStatus == READ_CANCEL) || (m_readStatus == READ_READY_SEL)) {
        // This is not a batch "continue" scan
        // check if we can modify the selection
        if ((m_optTlX != 0) && (m_optTlY != 0) &&
            (m_optBrX != 0) && (m_optBrY != 0))
        {
            // get maximums
            m_optBrX->getMaxValue(max_x);
            m_optBrY->getMaxValue(max_y);

            if (m_readStatus != READ_READY_SEL) {
                if (m_optTlX != 0) m_optTlX->storeCurrentData();
                if (m_optTlY != 0) m_optTlY->storeCurrentData();
                if (m_optBrX != 0) m_optBrX->storeCurrentData();
                if (m_optBrY != 0) m_optBrY->storeCurrentData();
                m_selIndex = ActiveSelection;
            }

            if (m_selIndex == ActiveSelection) {
                m_selIndex = 0;
                // try the active selection?
                m_previewViewer->activeSelection(x1,y1,x2,y2);
            }
            
            if ((x1==x2) || (y1==y2)) {
                // try a saved selection if the selection is invalid
                m_previewViewer->selectionAt(m_selIndex, x1,y1,x2,y2);
                kDebug() << m_selIndex;
                m_selIndex++;
            }

            // Select the whole area if the selection is broken.
            if ((x1==x2) || (y1==y2)) {
                x1=0; y1=0;
                x2=1; y2=1;
            }
            x1 *= max_x; y1 *= max_y;
            x2 *= max_x; y2 *= max_y;

            // now set the selection
            m_optTlX->setValue(x1);
            m_optTlY->setValue(y1);
            m_optBrX->setValue(x2);
            m_optBrY->setValue(y2);
        }
    }

    m_readStatus = READ_ON_GOING;

    // execute a pending value reload
    while (m_readValsTmr.isActive()) {
        m_readValsTmr.stop();
        valReload();
    }
    
    setBusy(true);
    
    m_startScanTmr.start(0);
}

void KSaneWidgetPrivate::startScan()
{
    SANE_Status status;
    // Start the scanning with sane_start
    status = sane_start(m_saneHandle);

    if (status != SANE_STATUS_GOOD) {
        switch (status)
        {
            #if !defined(SANE_CAP_ALWAYS_SETTABLE) && defined(SANE_STATUS_WARMING_UP) 
            // should better be done by detecting SANE's version in configure and providing a HAS_SANE_1_1
            // FIXME remove these ifdefs and require sane 1.1.x as soon as possible
            case SANE_STATUS_WARMING_UP:
                m_warmingUp->show();
                m_progressBar->hide();
                if (m_readStatus == READ_ON_GOING) {
                    m_startScanTmr.start(100);
                }
                else {
                    m_warmingUp->hide();
                    scanDone();
                }
                return;
            #endif
            case SANE_STATUS_NO_DOCS:
                // if it is the first doc in batch mode this is an error
                // do batch stuff here...
            case SANE_STATUS_JAMMED:
            case SANE_STATUS_COVER_OPEN:
            case SANE_STATUS_DEVICE_BUSY:
            case SANE_STATUS_ACCESS_DENIED:
            #if !defined(SANE_CAP_ALWAYS_SETTABLE) && defined(SANE_STATUS_HW_LOCKED) 
            case SANE_STATUS_HW_LOCKED:
            #endif
                KMessageBox::sorry(0, i18n(sane_strstatus(status)));
                scanCancel();
                setBusy(false);
                return;
            default:
                kDebug(51004) << "sane_start =" << status << "=" << sane_strstatus(status);
                scanCancel();
                setBusy(false);
                return;
        }
    }
    
    m_warmingUp->hide();
    m_progressBar->show();
    
    // Read image parameters
    status = sane_get_parameters(m_saneHandle, &m_params);
    if (status != SANE_STATUS_GOOD) {
        kDebug(51004) << "sane_get_parameters=" << sane_strstatus(status);
        scanCancel();
        setBusy(false);
        return;
    }
    
    // calculate data size
    m_frameSize  = m_params.lines * m_params.bytes_per_line;
    if ((m_params.format == SANE_FRAME_RED) ||
        (m_params.format == SANE_FRAME_GREEN) ||
        (m_params.format == SANE_FRAME_BLUE))
    {
        m_dataSize = m_frameSize*3;
    }
    else {
        m_dataSize = m_frameSize;
    }
    
    // make room for the new image
    if (m_isPreview) {
        // create a new image if necessary
        if ((m_previewImg.height() != m_params.lines) ||
            (m_previewImg.width() != m_params.pixels_per_line))
        {
            m_previewImg = QImage(m_params.pixels_per_line,
                                  m_params.lines,
                                  QImage::Format_RGB32);
            m_previewImg.fill(0xFFFFFFFF);
        }
        
        // update the size of the preview widget.
        m_previewViewer->setQImage(&m_previewImg);
        
        // update the size of the preview widget.
        m_previewViewer->zoom2Fit();
        
        // free unused buffer
        m_scanData.resize(0);
        
    }
    else {
        m_scanData.resize(m_dataSize);
    }
    
    m_pixel_x     = 0;
    m_pixel_y     = 0;
    m_frameRead   = 0;
    m_px_c_index  = 0;
    m_frame_t_count = 0;
    m_progressBar->setValue(0);
    
    m_readThread->start();
}

void KSaneWidgetPrivate::scanDone()
{
    if (m_isPreview) {
        m_previewViewer->updateImage();
        // even if the scan is finished successfully we need to call sane_cancel()
        sane_cancel(m_saneHandle);
        // restore the original settings of the changed parameters
        if (m_optDepth != 0) m_optDepth->restoreSavedData();
        if (m_optRes != 0) m_optRes->restoreSavedData();
        if (m_optResY != 0) m_optResY->restoreSavedData();
        if (m_optTlX != 0) m_optTlX->restoreSavedData();
        if (m_optTlY != 0) m_optTlY->restoreSavedData();
        if (m_optBrX != 0) m_optBrX->restoreSavedData();
        if (m_optBrY != 0) m_optBrY->restoreSavedData();
        if (m_optPreview != 0) m_optPreview->restoreSavedData();

        if (m_autoSelect) {
            m_previewViewer->findSelections();
        }
        m_readStatus = READ_READY;
        setBusy(false);
        return;
    }

    if (m_readStatus == READ_FINISHED) {
        // scan finished OK
        emit imageReady(m_scanData,
                         m_params.pixels_per_line,
                         m_params.lines,
                         getBytesPerLines(m_params),
                         (int)getImgFormat(m_params));

        if (m_readStatus == READ_ON_GOING) {
            // scanFinal() has been called in the imageReady slot.
            // a new "batch" scan is wanted.
            return;
        }
        
        // now check if we should have automatic ADF batch scaning
        QString source;
        if (m_optSource){
            m_optSource->getValue(source);
        }
        if (source == "Automatic Document Feeder") {
            // in batch mode only one area can be scanned per page
            kDebug() << "source == \"Automatic Document Feeder\"";
            scanFinal();
            return;
        }

        // not batch scan, call sane_cancel to be able to change parameters.
        sane_cancel(m_saneHandle);

        // check if we have multiple selections.
        if ((m_autoSelect == true) && (m_previewViewer->selListSize() > m_selIndex))
        {
            m_readStatus = READ_READY_SEL;
            scanFinal();
            return;
        }
    }
    
    sane_cancel(m_saneHandle);

    setBusy(false);
    if (m_optTlX != 0) m_optTlX->restoreSavedData();
    if (m_optTlY != 0) m_optTlY->restoreSavedData();
    if (m_optBrX != 0) m_optBrX->restoreSavedData();
    if (m_optBrY != 0) m_optBrY->restoreSavedData();

    bool error = (m_readStatus == READ_ERROR);
    m_readStatus = READ_READY;
    
    if (error) {
        emit scanDone(KSaneWidget::ErrorGeneral, "");
    }
    else {
        emit scanDone(KSaneWidget::NoError, "");
    }
}


void KSaneWidgetPrivate::scanCancel()
{
    m_readStatus = READ_CANCEL;
    m_progressBar->setValue(0);
}

void KSaneWidgetPrivate::setBusy(bool busy)
{
    m_optsTabWidget->setDisabled(busy);
    m_previewViewer->setDisabled(busy);
    m_zInBtn->setDisabled(busy);
    m_zOutBtn->setDisabled(busy);
    m_zSelBtn->setDisabled(busy);
    m_zFitBtn->setDisabled(busy);
    m_prevBtn->setDisabled(busy);
    m_scanBtn->setDisabled(busy);
    m_scanBtn->setFocus(Qt::OtherFocusReason);
    
    m_progressBar->setVisible(busy);
    m_cancelBtn->setVisible(busy);
    m_progressBar->setDisabled(!busy);
    m_cancelBtn->setDisabled(!busy);
}

#define inc_color_index(index) { index++; if (index==3) index=0;}

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

/** This slot is called when the read thread is finished.
* Here we check the status of the read and depending on the status either
* react on an error or go to the actual copying functions. */
void KSaneWidgetPrivate::processData()
{
    switch (m_readThread->status) {
        case SANE_STATUS_GOOD:
            // continue to parsing the data
            break;
            
        case SANE_STATUS_EOF:
            if (m_frameRead < m_frameSize) {
                kDebug(51004) << "frameRead =" << m_frameRead
                << ", frameSize =" << m_frameSize;
                m_readStatus = READ_ERROR;
                scanDone();
                return;
            }
            if (m_params.last_frame == SANE_TRUE) {
                // this is where it all ends well :)
                m_readStatus = READ_FINISHED;
                scanDone();
                return;
            }
            else {
                // start reading next frame
                SANE_Status status = sane_start(m_saneHandle);
                if (status != SANE_STATUS_GOOD) {
                    kDebug(51004) << "sane_start =" << sane_strstatus(status);
                    m_readStatus = READ_ERROR;
                    scanDone();
                    return;
                }
                status = sane_get_parameters(m_saneHandle, &m_params);
                if (status != SANE_STATUS_GOOD) {
                    kDebug(51004) << "sane_get_parameters =" << sane_strstatus(status);
                    m_readStatus = READ_ERROR;
                    scanDone();
                    return;
                }
                kDebug(51004) << "New Frame";
                m_frameRead = 0;
                m_frame_t_count++;
                break;
            }
        default:
            kDebug(51004) << "sane_read=" << m_readThread->status << "=" << sane_strstatus(m_readThread->status);
            m_readStatus = READ_ERROR;
            scanDone();
            return;
    }
    
    // copy the data to the buffer
    if (m_isPreview) {
        copyToPreview((int)m_readThread->readBytes);
    }
    else {
        copyToScanData((int)m_readThread->readBytes);
    }
    
    // update progressBar
    if (m_params.lines > 0) {
        long int new_progress;
        if ((m_params.format == SANE_FRAME_RED) ||
            (m_params.format == SANE_FRAME_GREEN) ||
            (m_params.format == SANE_FRAME_BLUE))
        {
            new_progress = (m_frame_t_count * m_frameSize) + m_frameRead;
        }
        else {
            new_progress = (m_frameRead);
        }
        new_progress = (int)((((float)new_progress / (float)m_dataSize) * 100.0) + 0.5);
        
        if (new_progress != m_progressBar->value()) {
            //kDebug(51004) << new_progress << m_frameRead << m_dataSize;
            if (m_isPreview && (m_timeSinceUpd.elapsed() > 300)) { // update the preview only every 300ms
                m_previewViewer->updateImage();
                m_timeSinceUpd.restart();
            }
            m_progressBar->setValue((int)new_progress);
            emit scanProgress(new_progress);
        }
        
    }
    
    if (m_readStatus == READ_ON_GOING) {
        m_readThread->start();
    }
    else {
        scanDone();
    }
}

void KSaneWidgetPrivate::copyToPreview(int read_bytes)
{
    int index;
    uchar *imgBits = m_previewImg.bits();
    switch (m_params.format)
    {
        case SANE_FRAME_GRAY:
            if (m_params.depth == 1) {
                int i, j;
                for (i=0; i<read_bytes; i++) {
                    for (j=7; j>=0; j--) {
                        if ((m_saneReadBuffer[i] & (1<<j)) == 0) {
                            m_previewImg.setPixel(m_pixel_x,
                                                   m_pixel_y,
                                                   qRgb(255,255,255));
                        }
                        else {
                            m_previewImg.setPixel(m_pixel_x,
                                                   m_pixel_y,
                                                   qRgb(0,0,0));
                        }
                        m_pixel_x++;
                        if(m_pixel_x >= m_params.pixels_per_line) {
                            m_pixel_x = 0;
                            m_pixel_y++;
                            break;
                        }
                        if (m_pixel_y >= m_params.lines) break;
                    }
                    m_frameRead++;
                }
                return;
            }
            else if (m_params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    index = m_frameRead * 4;
                    imgBits[index] = m_saneReadBuffer[i];
                    imgBits[index + 1] = m_saneReadBuffer[i];
                    imgBits[index + 2] = m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            else if (m_params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    if (m_frameRead%2 == 0) {
                        index = m_frameRead * 2;
                        imgBits[index] = m_saneReadBuffer[i+1];
                        imgBits[index + 1] = m_saneReadBuffer[i+1];
                        imgBits[index + 2] = m_saneReadBuffer[i+1];
                    }
                    m_frameRead++;
                }
                return;
            }
            break;
            
            case SANE_FRAME_RGB:
                if (m_params.depth == 8) {
                    for (int i=0; i<read_bytes; i++) {
                        m_px_colors[m_px_c_index] = m_saneReadBuffer[i];
                        inc_color_index(m_px_c_index);
                        m_frameRead++;
                        if (m_px_c_index == 0) {
                            m_previewImg.setPixel(m_pixel_x,
                                                  m_pixel_y,
                                                  qRgb(m_px_colors[0],
                                                       m_px_colors[1],
                                                       m_px_colors[2]));
                            inc_pixel(m_pixel_x, m_pixel_y, m_params.pixels_per_line);
                        }
                    }
                    return;
                }
                else if (m_params.depth == 16) {
                    for (int i=0; i<read_bytes; i++) {
                        m_frameRead++;
                        if (m_frameRead%2==0) {
                            m_px_colors[m_px_c_index] = m_saneReadBuffer[i];
                            inc_color_index(m_px_c_index);
                            if (m_px_c_index == 0) {
                                m_previewImg.setPixel(m_pixel_x,
                                                      m_pixel_y,
                                                      qRgb(m_px_colors[0],
                                                           m_px_colors[1],
                                                           m_px_colors[2]));
                                inc_pixel(m_pixel_x, m_pixel_y, m_params.pixels_per_line);
                            }
                        }
                    }
                    return;
                }
                break;
                
            case SANE_FRAME_RED:
                if (m_params.depth == 8) {
                    for (int i=0; i<read_bytes; i++) {
                        imgBits[index_red8_to_argb8(m_frameRead)] = m_saneReadBuffer[i];
                        m_frameRead++;
                    }
                    return;
                }
                else if (m_params.depth == 16) {
                    for (int i=0; i<read_bytes; i++) {
                        if (m_frameRead%2 == 0) {
                            imgBits[index_red16_to_argb8(m_frameRead)] = m_saneReadBuffer[i+1];
                        }
                        m_frameRead++;
                    }
                    return;
                }
                break;
                
            case SANE_FRAME_GREEN:
                if (m_params.depth == 8) {
                    for (int i=0; i<read_bytes; i++) {
                        imgBits[index_green8_to_argb8(m_frameRead)] = m_saneReadBuffer[i];
                        m_frameRead++;
                    }
                    return;
                }
                else if (m_params.depth == 16) {
                    for (int i=0; i<read_bytes; i++) {
                        if (m_frameRead%2 == 0) {
                            imgBits[index_green16_to_argb8(m_frameRead)] = m_saneReadBuffer[i+1];
                        }
                        m_frameRead++;
                    }
                    return;
                }
                break;
                
            case SANE_FRAME_BLUE:
                if (m_params.depth == 8) {
                    for (int i=0; i<read_bytes; i++) {
                        imgBits[index_blue8_to_argb8(m_frameRead)] = m_saneReadBuffer[i];
                        m_frameRead++;
                    }
                    return;
                }
                else if (m_params.depth == 16) {
                    for (int i=0; i<read_bytes; i++) {
                        if (m_frameRead%2 == 0) {
                            imgBits[index_blue16_to_argb8(m_frameRead)] = m_saneReadBuffer[i+1];
                        }
                        m_frameRead++;
                    }
                    return;
                }
                break;
    }
    
    KMessageBox::error(0, i18n("The image format is not (yet?) supported by libksane."));
    kDebug(51004) << "Format" << m_params.format
    << "and depth" << m_params.format
    << "is not yet suppoeted by libksane!";
    m_readStatus = READ_ERROR;
    return;
}

void KSaneWidgetPrivate::copyToScanData(int read_bytes)
{
    char *data = m_scanData.data();
    switch (m_params.format)
    {
        case SANE_FRAME_GRAY:
            memcpy(&(data[m_frameRead]), m_saneReadBuffer, read_bytes);
            m_frameRead += read_bytes;
            return;
        case SANE_FRAME_RGB:
            if (m_params.depth == 1) {
                break;
            }
            memcpy(&(data[m_frameRead]), m_saneReadBuffer, read_bytes);
            m_frameRead += read_bytes;
            return;
            
        case SANE_FRAME_RED:
            if (m_params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_red8_to_rgb8(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            else if (m_params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_red16_to_rgb16(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            break;
            
        case SANE_FRAME_GREEN:
            if (m_params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_green8_to_rgb8(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            else if (m_params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_green16_to_rgb16(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            break;
            
        case SANE_FRAME_BLUE:
            if (m_params.depth == 8) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_blue8_to_rgb8(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            else if (m_params.depth == 16) {
                for (int i=0; i<read_bytes; i++) {
                    data[index_blue16_to_rgb16(m_frameRead)] =
                    m_saneReadBuffer[i];
                    m_frameRead++;
                }
                return;
            }
            break;
    }
    
    KMessageBox::error(0, i18n("The image format is not (yet?) supported by libksane."));
    kDebug(51004) << "Format" << m_params.format
    << "and depth" << m_params.format
    << "is not yet suppoeted by libksane!";
    m_readStatus = READ_ERROR;
    return;
}

KSaneReadThread::KSaneReadThread(SANE_Handle handle, SANE_Byte *data, SANE_Int maxBytes):
QThread(),
status(SANE_STATUS_GOOD),
readBytes(0),
m_data(data),
m_maxBytes(maxBytes),
m_saneHandle(handle)
{}

void KSaneReadThread::run()
{
    status = sane_read(m_saneHandle, m_data, m_maxBytes, &readBytes);
}

}  // NameSpace KSaneIface
