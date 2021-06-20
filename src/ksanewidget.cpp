/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-24
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2009 Matthias Nagl <matthias at nagl dot info>
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanewidget.h"
#include "ksanewidget_p.h"

#include <unistd.h>

#include <QApplication>
#include <QVarLengthArray>
#include <QLabel>
#include <QSplitter>
#include <QMutex>
#include <QPointer>
#include <QIcon>
#include <QShortcut>

#include <kpassworddialog.h>
#ifdef HAVE_KF5WALLET
#include <kwallet.h>
#endif

#include "ksaneoption.h"
#include "ksaneoptbutton.h"
#include "ksaneoptcheckbox.h"
#include "ksaneoptcombo.h"
#include "ksaneoptentry.h"
#include "ksaneoptfslider.h"
#include "ksaneoptgamma.h"
#include "ksaneoptslider.h"
#include "ksanedevicedialog.h"
#include "labeledgamma.h"

#include <ksane_debug.h>

namespace KSaneIface
{
static int     s_objectCount = 0;
Q_GLOBAL_STATIC(QMutex, s_objectMutex)

static const QLatin1String InvetColorsOption("KSane::InvertColors");

KSaneWidget::KSaneWidget(QWidget *parent)
    : QWidget(parent), d(new KSaneWidgetPrivate(this))
{
    SANE_Int    version;
    SANE_Status status;

    s_objectMutex->lock();
    s_objectCount++;

    if (s_objectCount == 1) {
        // only call sane init for the first instance
        status = sane_init(&version, &KSaneAuth::authorization);
        if (status != SANE_STATUS_GOOD) {
            qCDebug(KSANE_LOG) << "libksane: sane_init() failed("
                     << sane_strstatus(status) << ")";
        } else {
            //qCDebug(KSANE_LOG) << "Sane Version = "
            //         << SANE_VERSION_MAJOR(version) << "."
            //         << SANE_VERSION_MINORparent(version) << "."
            //         << SANE_VERSION_BUILD(version);
        }
    }
    s_objectMutex->unlock();

    // read the device list to get a list of vendor and model info
    d->m_findDevThread->start();

    d->m_readValsTmr.setSingleShot(true);
    connect(&d->m_readValsTmr, SIGNAL(timeout()), d, SLOT(valReload()));

    d->m_updProgressTmr.setSingleShot(false);
    d->m_updProgressTmr.setInterval(300);
    connect(&d->m_updProgressTmr, SIGNAL(timeout()), d, SLOT(updateProgress()));

    // Create the static UI
    // create the preview
    d->m_previewViewer = new KSaneViewer(&(d->m_previewImg), this);
    connect(d->m_previewViewer, SIGNAL(newSelection(float,float,float,float)),
            d, SLOT(handleSelection(float,float,float,float)));

    d->m_warmingUp = new QLabel;
    d->m_warmingUp->setText(i18n("Waiting for the scan to start."));
    d->m_warmingUp->setAlignment(Qt::AlignCenter);
    d->m_warmingUp->setAutoFillBackground(true);
    d->m_warmingUp->setBackgroundRole(QPalette::Highlight);
    //d->m_warmingUp->setForegroundRole(QPalette::HighlightedText);
    d->m_warmingUp->hide();

    d->m_progressBar = new QProgressBar;
    d->m_progressBar->setMaximum(100);

    d->m_cancelBtn   = new QPushButton;
    d->m_cancelBtn->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    d->m_cancelBtn->setToolTip(i18n("Cancel current scan operation"));
    connect(d->m_cancelBtn, SIGNAL(clicked()), this, SLOT(scanCancel()));

    d->m_activityFrame = new QWidget;
    QHBoxLayout *progress_lay = new QHBoxLayout(d->m_activityFrame);
    progress_lay->setContentsMargins(0, 0, 0, 0);
    progress_lay->addWidget(d->m_progressBar, 100);
    progress_lay->addWidget(d->m_cancelBtn, 0);
    d->m_activityFrame->hide();

    d->m_zInBtn  = new QToolButton(this);
    d->m_zInBtn->setAutoRaise(true);
    d->m_zInBtn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    d->m_zInBtn->setToolTip(i18n("Zoom In"));
    connect(d->m_zInBtn, SIGNAL(clicked()), d->m_previewViewer, SLOT(zoomIn()));

    d->m_zOutBtn = new QToolButton(this);
    d->m_zOutBtn->setAutoRaise(true);
    d->m_zOutBtn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    d->m_zOutBtn->setToolTip(i18n("Zoom Out"));
    connect(d->m_zOutBtn, SIGNAL(clicked()), d->m_previewViewer, SLOT(zoomOut()));

    d->m_zSelBtn = new QToolButton(this);
    d->m_zSelBtn->setAutoRaise(true);
    d->m_zSelBtn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-fit-best")));
    d->m_zSelBtn->setToolTip(i18n("Zoom to Selection"));
    connect(d->m_zSelBtn, SIGNAL(clicked()), d->m_previewViewer, SLOT(zoomSel()));

    d->m_zFitBtn = new QToolButton(this);
    d->m_zFitBtn->setAutoRaise(true);
    d->m_zFitBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
    d->m_zFitBtn->setToolTip(i18n("Zoom to Fit"));
    connect(d->m_zFitBtn, SIGNAL(clicked()), d->m_previewViewer, SLOT(zoom2Fit()));

    d->m_clearSelBtn = new QToolButton(this);
    d->m_clearSelBtn->setAutoRaise(true);
    d->m_clearSelBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
    d->m_clearSelBtn->setToolTip(i18n("Clear Selections"));
    connect(d->m_clearSelBtn, SIGNAL(clicked()), d->m_previewViewer, SLOT(clearSelections()));

    QShortcut *prevShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+P")), this);
    connect(prevShortcut,   SIGNAL(activated()), d, SLOT(startPreviewScan()));

    QShortcut *scanShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+S")), this);
    connect(scanShortcut,   SIGNAL(activated()), d, SLOT(startFinalScan()));

    d->m_prevBtn = new QPushButton(this);
    d->m_prevBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    d->m_prevBtn->setToolTip(i18n("Scan Preview Image (%1)", prevShortcut->key().toString()));
    d->m_prevBtn->setText(i18nc("Preview button text", "Preview"));
    connect(d->m_prevBtn,   SIGNAL(clicked()), d, SLOT(startPreviewScan()));

    d->m_scanBtn = new QPushButton(this);
    d->m_scanBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    d->m_scanBtn->setToolTip(i18n("Scan Final Image (%1)", scanShortcut->key().toString()));
    d->m_scanBtn->setText(i18nc("Final scan button text", "Scan"));
    d->m_scanBtn->setFocus(Qt::OtherFocusReason);
    connect(d->m_scanBtn,   SIGNAL(clicked()), d, SLOT(startFinalScan()));

    d->m_btnFrame = new QWidget;
    QHBoxLayout *btn_lay = new QHBoxLayout(d->m_btnFrame);
    btn_lay->setContentsMargins(0, 0, 0, 0);
    btn_lay->addWidget(d->m_zInBtn);
    btn_lay->addWidget(d->m_zOutBtn);
    btn_lay->addWidget(d->m_zSelBtn);
    btn_lay->addWidget(d->m_zFitBtn);
    btn_lay->addWidget(d->m_clearSelBtn);
    btn_lay->addStretch(100);
    btn_lay->addWidget(d->m_prevBtn);
    btn_lay->addWidget(d->m_scanBtn);

    // calculate the height of the waiting/scanning/buttons frames to avoid jumpiness.
    int minHeight = d->m_btnFrame->sizeHint().height();
    if (d->m_activityFrame->sizeHint().height() > minHeight) {
        minHeight = d->m_activityFrame->sizeHint().height();
    }
    if (d->m_warmingUp->sizeHint().height() > minHeight) {
        minHeight = d->m_warmingUp->sizeHint().height();
    }
    d->m_btnFrame->setMinimumHeight(minHeight);
    d->m_activityFrame->setMinimumHeight(minHeight);
    d->m_warmingUp->setMinimumHeight(minHeight);

    d->m_previewFrame = new QWidget;
    QVBoxLayout *preview_layout = new QVBoxLayout(d->m_previewFrame);
    preview_layout->setContentsMargins(0, 0, 0, 0);
    preview_layout->addWidget(d->m_previewViewer, 100);
    preview_layout->addWidget(d->m_warmingUp, 0);
    preview_layout->addWidget(d->m_activityFrame, 0);
    preview_layout->addWidget(d->m_btnFrame, 0);

    // Create Options Widget
    d->m_optsTabWidget = new QTabWidget();

    // Add the basic options tab
    d->m_basicScrollA = new QScrollArea();
    d->m_basicScrollA->setWidgetResizable(true);
    d->m_basicScrollA->setFrameShape(QFrame::NoFrame);
    d->m_optsTabWidget->addTab(d->m_basicScrollA, i18n("Basic Options"));

    // Add the other options tab
    d->m_otherScrollA = new QScrollArea;
    d->m_otherScrollA->setWidgetResizable(true);
    d->m_otherScrollA->setFrameShape(QFrame::NoFrame);
    d->m_optsTabWidget->addTab(d->m_otherScrollA, i18n("Scanner Specific Options"));

    d->m_splitter = new QSplitter(this);
    d->m_splitter->addWidget(d->m_optsTabWidget);
    d->m_splitter->setStretchFactor(0, 0);
    d->m_splitter->addWidget(d->m_previewFrame);
    d->m_splitter->setStretchFactor(1, 100);

    d->m_optionsCollapser = new SplitterCollapser(d->m_splitter, d->m_optsTabWidget);

    QHBoxLayout *base_layout = new QHBoxLayout(this);
    base_layout->addWidget(d->m_splitter);
    base_layout->setContentsMargins(0, 0, 0, 0);

    // disable the interface in case no device is opened.
    d->m_optsTabWidget->setDisabled(true);
    d->m_previewViewer->setDisabled(true);
    d->m_btnFrame->setDisabled(true);

}

KSaneWidget::~KSaneWidget()
{
    while (!closeDevice()) {
        usleep(100);
    }
    // wait for any thread to exit

    s_objectMutex->lock();
    s_objectCount--;
    if (s_objectCount <= 0) {
        // only delete the find-devices and authorization singletons and call sane_exit
        // if this is the last instance
        delete d->m_findDevThread;
        delete d->m_auth;
        sane_exit();
    }
    s_objectMutex->unlock();
    delete d;
}

QString KSaneWidget::deviceName() const
{
    return d->m_devName;
}

QString KSaneWidget::deviceVendor() const
{
    return d->m_vendor;
}

QString KSaneWidget::deviceModel() const
{
    return d->m_model;
}

QString KSaneWidget::vendor() const
{
    d->m_findDevThread->wait();
    d->devListUpdated(); // this is just a wrapped if (m_vendor.isEmpty()) statement if the vendor is known
    // devListUpdated here is to ensure that we do not come in between finished and the devListUpdated slot
    return d->m_vendor;
}

QString KSaneWidget::make() const
{
    return vendor();
}

QString KSaneWidget::model() const
{
    d->m_findDevThread->wait();
    d->devListUpdated(); // this is just a wrapped if (m_vendor.isEmpty()) statement if the vendor is known
    // devListUpdated here is to ensure that we do not come in between finished and the devListUpdated slot
    return d->m_model;
}

QString KSaneWidget::selectDevice(QWidget *parent)
{
    QString selected_name;
    QPointer<KSaneDeviceDialog> sel = new KSaneDeviceDialog(parent);

    // set default scanner - perhaps application using libksane should remember that
    // 2014-01-21: gm: +1
    // sel.setDefault(prev_backend);

    if (sel->exec() == QDialog::Accepted) {
        selected_name = sel->getSelectedName();
    }

    delete sel;
    return selected_name;
}

void KSaneWidget::initGetDeviceList() const
{
    /* On some SANE backends, the handle becomes invalid when
     * querying for new devices. Hence, this is only allowed when
     * no device is currently opened. */
    if (d->m_saneHandle == nullptr) {
        d->m_findDevThread->start();
    }
}

bool KSaneWidget::openDevice(const QString &deviceName)
{
    int                            i = 0;
    const SANE_Option_Descriptor  *optDesc;
    SANE_Status                    status;
    SANE_Word                      numSaneOptions;
    SANE_Int                       res;
    KPasswordDialog               *dlg;
#ifdef HAVE_KF5WALLET
    KWallet::Wallet               *saneWallet;
#endif
    QString                        myFolderName = QStringLiteral("ksane");
    QMap<QString, QString>         wallet_entry;

    if (d->m_saneHandle != nullptr) {
        // this KSaneWidget already has an open device
        return false;
    }

    // don't bother trying to open if the device string is empty
    if (deviceName.isEmpty()) {
        return false;
    }
    // save the device name
    d->m_devName = deviceName;

    // Try to open the device
    status = sane_open(deviceName.toLatin1().constData(), &d->m_saneHandle);

    bool password_dialog_ok = true;

    // prepare wallet for authentication and create password dialog
    if (status == SANE_STATUS_ACCESS_DENIED) {
#ifdef HAVE_KF5WALLET
        saneWallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), winId());

        if (saneWallet) {
            dlg = new KPasswordDialog(this, KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword);
            if (!saneWallet->hasFolder(myFolderName)) {
                saneWallet->createFolder(myFolderName);
            }
            saneWallet->setFolder(myFolderName);
            saneWallet->readMap(deviceName, wallet_entry);
            if (!wallet_entry.empty() || true) {
                dlg->setUsername(wallet_entry[QStringLiteral("username")]);
                dlg->setPassword(wallet_entry[QStringLiteral("password")]);
                dlg->setKeepPassword(true);
            }
        } else
#endif
        {
            dlg = new KPasswordDialog(this, KPasswordDialog::ShowUsernameLine);
        }
        dlg->setPrompt(i18n("Authentication required for resource: %1", deviceName));

    }

    // sane_open failed due to insufficient authorization
    // retry opening device with user provided data assisted with kwallet records
    while (status == SANE_STATUS_ACCESS_DENIED) {

        password_dialog_ok = dlg->exec();
        if (!password_dialog_ok) {
            delete dlg;
            d->m_devName.clear();
            return false; //the user canceled
        }

        // add/update the device user-name and password for authentication
        d->m_auth->setDeviceAuth(d->m_devName, dlg->username(), dlg->password());

        status = sane_open(deviceName.toLatin1().constData(), &d->m_saneHandle);

#ifdef HAVE_KF5WALLET
        // store password in wallet on successful authentication
        if (dlg->keepPassword() && status != SANE_STATUS_ACCESS_DENIED) {
            QMap<QString, QString> entry;
            entry[QStringLiteral("username")] = dlg->username();
            entry[QStringLiteral("password")] = dlg->password();
            if (saneWallet) {
                saneWallet->writeMap(deviceName, entry);
            }
        }
#endif
    }

    if (status != SANE_STATUS_GOOD) {
        qCDebug(KSANE_LOG) << "sane_open(\"" << deviceName << "\", &handle) failed! status = " << sane_strstatus(status);
        d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_devName.clear();
        return false;
    }

    // update the device list if needed to get the vendor and model info
    if (d->m_findDevThread->devicesList().size() == 0) {
        d->m_findDevThread->start();
    } else {
        // use the "old" existing list
        d->devListUpdated();
        // if m_vendor is not updated it means that the list needs to be updated.
        if (d->m_vendor.isEmpty()) {
            d->m_findDevThread->start();
        }
    }

    // Read the options (start with option 0 the number of parameters)
    optDesc = sane_get_option_descriptor(d->m_saneHandle, 0);
    if (optDesc == nullptr) {
        d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_devName.clear();
        return false;
    }
    QVarLengthArray<char> data(optDesc->size);
    status = sane_control_option(d->m_saneHandle, 0, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        d->m_auth->clearDeviceAuth(d->m_devName);
        d->m_devName.clear();
        return false;
    }
    numSaneOptions = *reinterpret_cast<SANE_Word *>(data.data());

    // read the rest of the options
    for (i = 1; i < numSaneOptions; ++i) {
        switch (KSaneOption::optionType(sane_get_option_descriptor(d->m_saneHandle, i))) {
        case KSaneOption::TYPE_DETECT_FAIL:
            d->m_optList.append(new KSaneOption(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_CHECKBOX:
            d->m_optList.append(new KSaneOptCheckBox(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_SLIDER:
            d->m_optList.append(new KSaneOptSlider(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_F_SLIDER:
            d->m_optList.append(new KSaneOptFSlider(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_COMBO:
            d->m_optList.append(new KSaneOptCombo(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_ENTRY:
            d->m_optList.append(new KSaneOptEntry(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_GAMMA:
            d->m_optList.append(new KSaneOptGamma(d->m_saneHandle, i));
            break;
        case KSaneOption::TYPE_BUTTON:
            d->m_optList.append(new KSaneOptButton(d->m_saneHandle, i));
            break;
        }
    }

    // do the connections of the option parameters
    for (i = 0; i < d->m_optList.size(); ++i) {
        //qCDebug(KSANE_LOG) << d->m_optList.at(i)->name();
        connect(d->m_optList.at(i), SIGNAL(optsNeedReload()), d, SLOT(optReload()));
        connect(d->m_optList.at(i), SIGNAL(valsNeedReload()), d, SLOT(scheduleValReload()));

        if (d->m_optList.at(i)->needsPolling()) {
            //qCDebug(KSANE_LOG) << d->m_optList.at(i)->name() << " needs polling";
            d->m_pollList.append(d->m_optList.at(i));
            KSaneOptCheckBox *buttonOption = qobject_cast<KSaneOptCheckBox *>(d->m_optList.at(i));
            if (buttonOption) {
                connect(buttonOption, SIGNAL(buttonPressed(QString,QString,bool)),
                        this, SIGNAL(buttonPressed(QString,QString,bool)));
            }
        }
    }

    // start polling the poll options
    if (d->m_pollList.size() > 0) {
        d->m_optionPollTmr.start();
    }

    // Create the preview thread
    d->m_previewThread = new KSanePreviewThread(d->m_saneHandle, &d->m_previewImg);
    connect(d->m_previewThread, SIGNAL(finished()), d, SLOT(previewScanDone()));

    // Create the read thread
    d->m_scanThread = new KSaneScanThread(d->m_saneHandle, &d->m_scanData);
    connect(d->m_scanThread, SIGNAL(finished()), d, SLOT(oneFinalScanDone()));

    // Create the options interface
    d->createOptInterface();

    // try to set KSaneWidget default values
    d->setDefaultValues();

    // Enable the interface
    d->m_optsTabWidget->setDisabled(false);
    d->m_previewViewer->setDisabled(false);
    d->m_btnFrame->setDisabled(false);

    // estimate the preview size and create an empty image
    // this is done so that you can select scan area without
    // having to scan a preview.
    d->updatePreviewSize();
    QTimer::singleShot(1000, d->m_previewViewer, SLOT(zoom2Fit()));
    return true;
}

bool KSaneWidget::closeDevice()
{
    if (!d->m_saneHandle) {
        return true;
    }

    if (d->m_scanThread->isRunning()) {
        d->m_scanThread->cancelScan();
        d->m_closeDevicePending = true;
        return false;
    }

    if (d->m_previewThread->isRunning()) {
        d->m_previewThread->cancelScan();
        d->m_closeDevicePending = true;
        return false;
    }

    d->m_auth->clearDeviceAuth(d->m_devName);
    // else
    sane_close(d->m_saneHandle);
    d->m_saneHandle = nullptr;
    d->clearDeviceOptions();

    // disable the interface until a new device is opened.
    d->m_optsTabWidget->setDisabled(true);
    d->m_previewViewer->setDisabled(true);
    d->m_btnFrame->setDisabled(true);

    return true;
}

#define inc_pixel(x,y,ppl) { x++; if (x>=ppl) { y++; x=0;} }

QImage KSaneWidget::toQImageSilent(const QByteArray &data,
                                   int width,
                                   int height,
                                   int bytes_per_line,
                                   ImageFormat format)
{
    return KSaneWidget::toQImageSilent(data, width, height, bytes_per_line, currentDPI(), format);
}

QImage KSaneWidget::toQImageSilent(const QByteArray &data,
                                   int width,
                                   int height,
                                   int bytes_per_line,
                                   int dpi,
                                   ImageFormat format)
{
    QImage img;
    int j = 0;
    QVector<QRgb> table;
    QRgb *imgLine;

    switch (format) {
    case FormatBlackWhite:
        img = QImage((uchar *)data.data(),
                     width,
                     height,
                     bytes_per_line,
                     QImage::Format_Mono);
        // The color table must be set
        table.append(0xFFFFFFFF);
        table.append(0xFF000000);
        img.setColorTable(table);
        break;

    case FormatGrayScale8: {
        img = QImage(width, height, QImage::Format_RGB32);
        int dI = 0;
        for (int i = 0; (i < img.height() && dI < data.size()); i++) {
            imgLine = reinterpret_cast<QRgb *>(img.scanLine(i));
            for (j = 0; (j < img.width() && dI < data.size()); j++) {
                imgLine[j] = qRgb(data[dI], data[dI], data[dI]);
                dI++;
            }
        }
        break;
    }
    case FormatGrayScale16: {
        img = QImage(width, height, QImage::Format_RGB32);
        int dI = 1;
        for (int i = 0; (i < img.height() && dI < data.size()); i++) {
            imgLine = reinterpret_cast<QRgb *>(img.scanLine(i));
            for (j = 0; (j < img.width() && dI < data.size()); j++) {
                imgLine[j] = qRgb(data[dI], data[dI], data[dI]);
                dI += 2;
            }
        }
        break;
    }
    case FormatRGB_8_C: {
        img = QImage(width, height, QImage::Format_RGB32);
        int dI = 0;
        for (int i = 0; (i < img.height() && dI < data.size()); i++) {
            imgLine = reinterpret_cast<QRgb *>(img.scanLine(i));
            for (j = 0; (j < img.width() && dI < data.size()); j++) {
                imgLine[j] = qRgb(data[dI], data[dI + 1], data[dI + 2]);
                dI += 3;
            }
        }
        break;
    }
    case FormatRGB_16_C: {
        img = QImage(width, height, QImage::Format_RGB32);
        int dI = 1;
        for (int i = 0; (i < img.height() && dI < data.size()); i++) {
            imgLine = reinterpret_cast<QRgb *>(img.scanLine(i));
            for (j = 0; (j < img.width() && dI < data.size()); j++) {
                imgLine[j] = qRgb(data[dI], data[dI + 2], data[dI + 4]);
                dI += 6;
            }
        }
        break;
    }
    case FormatNone:
    default:
        qCDebug(KSANE_LOG) << "Unsupported conversion";
        break;
    }
    float dpm = dpi * (1000.0 / 25.4);
    img.setDotsPerMeterX(dpm);
    img.setDotsPerMeterY(dpm);
    return img;
}

QImage KSaneWidget::toQImage(const QByteArray &data,
                             int width,
                             int height,
                             int bytes_per_line,
                             ImageFormat format)
{

    if ((format == FormatRGB_16_C) || (format == FormatGrayScale16)) {
        d->alertUser(KSaneWidget::ErrorGeneral, i18n("The image data contained 16 bits per color, "
                     "but the color depth has been truncated to 8 bits per color."));
    }
    return toQImageSilent(data, width, height, bytes_per_line, format);
}

void KSaneWidget::scanFinal()
{
    if (d->m_btnFrame->isEnabled()) {
        d->startFinalScan();
    } else {
        // if the button frame is disabled, there is no open device to scan from
        Q_EMIT scanDone(KSaneWidget::ErrorGeneral, QString());
    }
}

void KSaneWidget::startPreviewScan()
{
    if (d->m_btnFrame->isEnabled()) {
        d->startPreviewScan();
    } else {
        // if the button frame is disabled, there is no open device to scan from
        Q_EMIT scanDone(KSaneWidget::ErrorGeneral, QString());
    }
}

void KSaneWidget::scanCancel()
{
    if (d->m_scanThread->isRunning()) {
        d->m_scanThread->cancelScan();
    }

    if (d->m_previewThread->isRunning()) {
        d->m_previewThread->cancelScan();
    }
    d->m_cancelMultiScan = true;
}

void KSaneWidget::setPreviewResolution(float dpi)
{
    d->m_previewDPI = dpi;
}

void KSaneWidget::getOptVals(QMap <QString, QString> &opts)
{
    KSaneOption *option;
    opts.clear();
    QString tmp;

    for (int i = 0; i < d->m_optList.size(); i++) {
        option = d->m_optList.at(i);
        if (option->getValue(tmp)) {
            opts[option->name()] = tmp;
        }
    }
    // Special handling for non sane option
    opts[InvetColorsOption] = d->m_invertColors->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
}

bool KSaneWidget::getOptVal(const QString &optname, QString &value)
{
    KSaneOption *option;

    if ((option = d->getOption(optname)) != nullptr) {
        return option->getValue(value);
    }
    // Special handling for non sane option
    if (optname == InvetColorsOption) {
        value = d->m_invertColors->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
        return true;
    }
    return false;
}

int KSaneWidget::setOptVals(const QMap <QString, QString> &opts)
{
    if (d->m_scanThread->isRunning() ||
            d->m_previewThread->isRunning()) {
        return -1;
    }

    QMap <QString, QString> optionMapCopy = opts;

    QString tmp;
    int i;
    int ret = 0;

    const QString sourceOptionSaneName = QStringLiteral(SANE_NAME_SCAN_SOURCE);
    const QString modeOptionSaneName   = QStringLiteral(SANE_NAME_SCAN_MODE);

    KSaneOption *opt;

    // Priorize source option
    if (optionMapCopy.contains(sourceOptionSaneName)) {
        if ((opt = d->getOption(sourceOptionSaneName)) != nullptr) {
            if (opt->setValue(optionMapCopy[sourceOptionSaneName])) {
                ret++;
            }
        }
        optionMapCopy.remove(sourceOptionSaneName);
    }

    // Priorize mode option
    if (optionMapCopy.contains(modeOptionSaneName)) {
        if ((opt = d->getOption(modeOptionSaneName)) != nullptr) {
            if (opt->setValue(optionMapCopy[modeOptionSaneName])) {
                ret++;
            }
        }
        optionMapCopy.remove(modeOptionSaneName);
    }

    // Update remaining options
    for (i = 0; i < d->m_optList.size(); i++) {
        if (optionMapCopy.contains(d->m_optList.at(i)->name())) {
            tmp = optionMapCopy[d->m_optList.at(i)->name()];
            if (d->m_optList.at(i)->setValue(tmp)) {
                ret++;
            }
        }
    }
    if ((d->m_splitGamChB) &&
            (d->m_optGamR) &&
            (d->m_optGamG) &&
            (d->m_optGamB)) {
        // check if the current gamma values are identical. if they are identical,
        // uncheck the "Separate color intensity tables" checkbox
        QString redGamma;
        QString greenGamma;
        QString blueGamma;
        d->m_optGamR->getValue(redGamma);
        d->m_optGamG->getValue(greenGamma);
        d->m_optGamB->getValue(blueGamma);
        if ((redGamma == greenGamma) && (greenGamma == blueGamma)) {
            d->m_splitGamChB->setChecked(false);
            // set the values to the common gamma widget
            d->m_commonGamma->setValues(redGamma);
        } else {
            d->m_splitGamChB->setChecked(true);
        }
    }

    // special handling for non-sane option
    if (optionMapCopy.contains(InvetColorsOption)) {
        tmp = optionMapCopy[InvetColorsOption];
        if ((tmp.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) ||
                (tmp.compare(QStringLiteral("1")) == 0)) {
            d->m_invertColors->setChecked(true);
        } else {
            d->m_invertColors->setChecked(false);
        }
    }
    return ret;
}

bool KSaneWidget::setOptVal(const QString &option, const QString &value)
{
    if (d->m_scanThread->isRunning() ||
            d->m_previewThread->isRunning()) {
        return false;
    }

    KSaneOption *opt;

    if ((opt = d->getOption(option)) != nullptr) {
        if (opt->setValue(value)) {
            if ((d->m_splitGamChB) &&
                    (d->m_optGamR) &&
                    (d->m_optGamG) &&
                    (d->m_optGamB) &&
                    ((opt == d->m_optGamR) ||
                     (opt == d->m_optGamG) ||
                     (opt == d->m_optGamB))) {
                // check if the current gamma values are identical. if they are identical,
                // uncheck the "Separate color intensity tables" checkbox
                QString redGamma;
                QString greenGamma;
                QString blueGamma;
                d->m_optGamR->getValue(redGamma);
                d->m_optGamG->getValue(greenGamma);
                d->m_optGamB->getValue(blueGamma);
                if ((redGamma == greenGamma) && (greenGamma == blueGamma)) {
                    d->m_splitGamChB->setChecked(false);
                    // set the values to the common gamma widget
                    d->m_commonGamma->setValues(redGamma);
                } else {
                    d->m_splitGamChB->setChecked(true);
                }
            }
            return true;
        }
    }

    // special handling for non-sane option
    if (option == InvetColorsOption) {
        if ((value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) ||
                (value.compare(QStringLiteral("1")) == 0)) {
            d->m_invertColors->setChecked(true);
        } else {
            d->m_invertColors->setChecked(false);
        }
        return true;
    }

    return false;
}

void KSaneWidget::setScanButtonText(const QString &scanLabel)
{
    if (d->m_scanBtn == nullptr) {
        qCritical() << "setScanButtonText was called before KSaneWidget was initialized";
        return;
    }
    d->m_scanBtn->setText(scanLabel);
}

void KSaneWidget::setPreviewButtonText(const QString &previewLabel)
{
    if (d->m_scanBtn == nullptr) {
        qCritical() << "setPreviewButtonText was called before KSaneWidget was initialized";
        return;
    }
    d->m_prevBtn->setText(previewLabel);
}

void KSaneWidget::enableAutoSelect(bool enable)
{
    d->m_autoSelect = enable;
}

float KSaneWidget::currentDPI()
{
    if (d->m_optRes) {
        float value;
        if (d->m_optRes->getValue(value)) {
            return value;
        }
    }
    return 0.0; // Failure to read DPI
}

float KSaneWidget::scanAreaWidth()
{
    float result = 0.0;
    if (d->m_optBrX) {
        if (d->m_optBrX->getUnit() == SANE_UNIT_PIXEL) {
            d->m_optBrX->getMaxValue(result);
            float dpi = currentDPI();
            if (dpi < 1) {
                qCDebug(KSANE_LOG) << "Broken DPI value";
                dpi = 1.0;
            }
            result = result / dpi / 25.4;
        } else if (d->m_optBrX->getUnit() == SANE_UNIT_MM) {
            d->m_optBrX->getMaxValue(result);
        }
    }
    return result;
}

float KSaneWidget::scanAreaHeight()
{
    float result = 0.0;
    if (d->m_optBrY) {
        if (d->m_optBrY->getUnit() == SANE_UNIT_PIXEL) {
            d->m_optBrY->getMaxValue(result);
            float dpi = currentDPI();
            if (dpi < 1) {
                qCDebug(KSANE_LOG) << "Broken DPI value";
                dpi = 1.0;
            }
            result = result / dpi / 25.4;
        } else if (d->m_optBrY->getUnit() == SANE_UNIT_MM) {
            d->m_optBrY->getMaxValue(result);
        }
    }
    return result;
}

void KSaneWidget::setSelection(QPointF topLeft, QPointF bottomRight)
{
    if (!d->m_optBrX || !d->m_optBrY || !d->m_optTlX || !d->m_optTlY) {
        return;
    }
    if (topLeft.x() < 0.0 || topLeft.y() < 0.0 || bottomRight.x() < 0.0 || bottomRight.y() < 0.0) {
        d->m_previewViewer->clearActiveSelection();
        return;
    }

    float tlxRatio = d->scanAreaToRatioX(topLeft.x());
    float tlyRatio = d->scanAreaToRatioY(topLeft.y());
    float brxRatio = d->scanAreaToRatioX(bottomRight.x());
    float bryRatio = d->scanAreaToRatioX(bottomRight.y());

    d->m_previewViewer->setSelection(tlxRatio, tlyRatio, brxRatio, bryRatio);
}

void KSaneWidget::setOptionsCollapsed(bool collapse)
{
    if (collapse) {
        QTimer::singleShot(0, d->m_optionsCollapser, SLOT(slotCollapse()));
    } else {
        QTimer::singleShot(0, d->m_optionsCollapser, SLOT(slotRestore()));
    }
}

void KSaneWidget::setScanButtonHidden(bool hidden)
{
    d->m_scanBtn->setHidden(hidden);
}

}  // NameSpace KSaneIface
