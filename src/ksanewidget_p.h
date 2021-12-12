/*
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef KSANE_WIDGET_PRIVATE_H
#define KSANE_WIDGET_PRIVATE_H

#include <QWidget>
#include <QCheckBox>
#include <QTimer>
#include <QProgressBar>
#include <QTabWidget>
#include <QPushButton>
#include <QVector>
#include <QSplitter>
#include <QToolButton>
#include <QSet>

#include <CoreInterface>
#include <CoreOption>

#include "ksanewidget.h"
#include "ksaneoptionwidget.h"
#include "ksaneviewer.h"
#include "labeledcombo.h"
#include "labeledfslider.h"
#include "labeledentry.h"
#include "ksanebutton.h"
#include "labeledgamma.h"
#include "labeledcheckbox.h"
#include "splittercollapser.h"

#define IMG_DATA_R_SIZE 100000

/** This namespace collects all methods and classes in LibKSane. */
namespace KSaneIface
{
class KSaneWidgetPrivate: public QObject
{
    Q_OBJECT

public:
    KSaneWidgetPrivate(KSaneWidget *);
    void clearDeviceOptions();
    void createOptInterface();
    void updatePreviewSize();
    void setBusy(bool busy);
    KSaneOptionWidget *createOptionWidget(QWidget *parent, KSane::CoreOption *option);
    KSaneWidget::ImageFormat getImgFormat(const QImage &image);

    float ratioToScanAreaX(float ratio);
    float ratioToScanAreaY(float ratio);
    float scanAreaToRatioX(float scanArea);
    float scanAreaToRatioY(float scanArea);

    float ratioToDispUnitX(float ratio);
    float ratioToDispUnitY(float ratio);
    float dispUnitToRatioX(float mm);
    float dispUnitToRatioY(float mm);

    bool scanSourceADF();

public Q_SLOTS:
    void startFinalScan();
    void startPreviewScan();
    void scanDone(KSane::CoreInterface::ScanStatus status, const QString &strStatus);
    void previewScanDone(KSane::CoreInterface::ScanStatus status, const QString &strStatus);
    void oneFinalScanDone(KSane::CoreInterface::ScanStatus status, const QString &strStatus);
    void updateProgress(int progress);
    void updateCountDown(int remainingSeconds);
    void handleSelection(float tl_x, float tl_y, float br_x, float br_y);
    void signalDevListUpdate(const QList<KSane::DeviceInformation*> &deviceList);
    void imageReady(const QImage &image);

private Q_SLOTS:

    void setTLX(const QVariant &x);
    void setTLY(const QVariant &y);
    void setBRX(const QVariant &x);
    void setBRY(const QVariant &y);

    void checkInvert();
    void invertPreview();

    void updateScanSelection();
    void setPossibleScanSizes();
    void setPageSize(int index);

    void updateCommonGamma();
    void updatePreviewViewer();

public:
    void alertUser(KSane::CoreInterface::ScanStatus status, const QString &strStatus);

public:
    KSane::CoreInterface   *m_ksaneCoreInterface;

    // backend independent
    QTabWidget         *m_optsTabWidget;
    QScrollArea        *m_basicScrollA;
    QWidget            *m_basicOptsTab;
    QWidget            *m_colorOpts;
    QScrollArea        *m_advancedScrollA;
    QWidget            *m_advancedOptsTab;
    QScrollArea        *m_otherScrollA;
    QWidget            *m_otherOptsTab;

    QVector<int>        m_sizeCodes;
    LabeledCombo       *m_scanareaPapersize;
    bool                m_settingPageSize = false;
    LabeledFSlider     *m_scanareaWidth;
    LabeledFSlider     *m_scanareaHeight;
    LabeledFSlider     *m_scanareaX;
    LabeledFSlider     *m_scanareaY;

    QSplitter          *m_splitter;
    SplitterCollapser  *m_optionsCollapser;

    QWidget            *m_previewFrame;
    KSaneViewer        *m_previewViewer;
    QWidget            *m_btnFrame;
    QToolButton        *m_zInBtn;
    QToolButton        *m_zOutBtn;
    QToolButton        *m_zSelBtn;
    QToolButton        *m_zFitBtn;
    QToolButton        *m_clearSelBtn;
    QPushButton        *m_scanBtn;
    QPushButton        *m_prevBtn;

    QWidget            *m_activityFrame;
    QLabel             *m_warmingUp;
    QLabel             *m_countDown;
    QProgressBar       *m_progressBar;
    QPushButton        *m_cancelBtn;

    // Option variables
    QSet<QString>            m_handledOptions;
    KSane::CoreOption        *m_optSource;
    KSane::CoreOption        *m_optNegative;
    KSane::CoreOption        *m_optFilmType;
    KSane::CoreOption        *m_optMode;
    KSane::CoreOption        *m_optDepth;
    KSane::CoreOption        *m_optRes;
    KSane::CoreOption        *m_optResX;
    KSane::CoreOption        *m_optResY;
    KSane::CoreOption        *m_optTlX;
    KSane::CoreOption        *m_optTlY;
    KSane::CoreOption        *m_optBrX;
    KSane::CoreOption        *m_optBrY;
    KSane::CoreOption        *m_optPreview;
    KSane::CoreOption        *m_optGamR;
    KSane::CoreOption        *m_optGamG;
    KSane::CoreOption        *m_optGamB;
    KSane::CoreOption        *m_optInvert;
    LabeledCheckbox    *m_splitGamChB;
    LabeledGamma       *m_commonGamma;
    KSane::CoreOption        *m_optWaitForBtn;

    // preview variables
    float               m_previewWidth;
    float               m_previewHeight;
    float               m_previewDPI;
    QImage              m_previewImg;
    bool                m_isPreview;
    bool                m_autoSelect;

    bool                m_cancelMultiScan = false;
    bool                m_scanOngoing = false;
    int                 m_selIndex;

    KSaneWidget           *q;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
