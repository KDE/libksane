/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_WIDGET_PRIVATE_H
#define KSANE_WIDGET_PRIVATE_H

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

#include <QWidget>
#include <QCheckBox>
#include <QTimer>
#include <QTime>
#include <QProgressBar>
#include <QTabWidget>
#include <QPushButton>
#include <QVector>
#include <QSplitter>
#include <QToolButton>
#include <QSet>

#include "ksanewidget.h"
#include "ksaneoption.h"
#include "ksaneinvertoption.h"
#include "ksaneoptionwidget.h"
#include "ksaneviewer.h"
#include "labeledcombo.h"
#include "labeledfslider.h"
#include "labeledentry.h"
#include "ksanebutton.h"
#include "labeledgamma.h"
#include "labeledcheckbox.h"
#include "splittercollapser.h"
#include "ksanescanthread.h"
#include "ksanepreviewthread.h"
#include "ksanefinddevicesthread.h"
#include "ksaneauth.h"

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
    void setDefaultValues();
    void setBusy(bool busy);
    KSaneOption *getOption(const QString &name);
    KSaneOptionWidget *createOptionWidget(QWidget *parent, KSaneOption *option);
    KSaneWidget::ImageFormat getImgFormat(SANE_Parameters &params);
    int getBytesPerLines(SANE_Parameters &params);

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
    void devListUpdated();
    void signalDevListUpdate();
    void startFinalScan();
    void startPreviewScan();
    void previewScanDone();
    void oneFinalScanDone();
    void updateProgress();
    void scheduleValuesReload();
    void reloadOptions();
    void reloadValues();
    void handleSelection(float tl_x, float tl_y, float br_x, float br_y);
    
private Q_SLOTS:

    void setTLX(const QVariant &x);
    void setTLY(const QVariant &y);
    void setBRX(const QVariant &x);
    void setBRY(const QVariant &y);

    void checkInvert();
    void invertPreview();
    void pollPollOptions();

    void updateScanSelection();
    void setPossibleScanSizes();
    void setPageSize(int index);

public:
    void alertUser(int type, const QString &strStatus);

public:
    // backend independent
    QTabWidget         *m_optsTabWidget;
    QScrollArea        *m_basicScrollA;
    QWidget            *m_basicOptsTab;
    QWidget            *m_colorOpts;
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
    QProgressBar       *m_progressBar;
    QPushButton        *m_cancelBtn;

    // device info
    SANE_Handle         m_saneHandle;
    QString             m_devName;
    QString             m_vendor;
    QString             m_model;

    // Option variables
    QList<KSaneOption *> m_optList;
    QList<KSaneOption *> m_pollList;
    QSet<QString>        m_optWithWidget;
    KSaneOption        *m_optSource;
    KSaneOption        *m_optNegative;
    KSaneOption        *m_optFilmType;
    KSaneOption        *m_optMode;
    KSaneOption        *m_optDepth;
    KSaneOption        *m_optRes;
    KSaneOption        *m_optResX;
    KSaneOption        *m_optResY;
    KSaneOption        *m_optTlX;
    KSaneOption        *m_optTlY;
    KSaneOption        *m_optBrX;
    KSaneOption        *m_optBrY;
    KSaneOption        *m_optPreview;
    KSaneOption        *m_optGamR;
    KSaneOption        *m_optGamG;
    KSaneOption        *m_optGamB;
    KSaneOption        *m_optInvert;
    LabeledCheckbox    *m_splitGamChB;
    LabeledGamma       *m_commonGamma;
    KSaneOption        *m_optWaitForBtn;

    // preview variables
    float               m_previewWidth;
    float               m_previewHeight;
    float               m_previewDPI;
    QImage              m_previewImg;
    bool                m_isPreview;
    bool                m_autoSelect;

    int                 m_selIndex;

    bool                m_scanOngoing;
    bool                m_closeDevicePending;
    bool                m_cancelMultiScan = false;

    // final image data
    QByteArray          m_scanData;

    // option handling
    QTimer              m_readValsTmr;
    QTimer              m_updProgressTmr;
    QTimer              m_optionPollTmr;
    KSaneScanThread    *m_scanThread;
    KSanePreviewThread *m_previewThread;

    QString             m_saneUserName;
    QString             m_sanePassword;

    FindSaneDevicesThread *m_findDevThread;
    KSaneAuth             *m_auth;
    KSaneWidget           *q;
};

}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
