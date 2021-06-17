/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2010 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2009 Matthias Nagl <matthias at nagl dot info>
 * SPDX-FileCopyrightText: 2009 Grzegorz Kurtyka <grzegorz dot kurtyka at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 * 
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanewidget.h"
#include "ksanewidget_p.h"

#include <unistd.h>

#include <QApplication>
#include <QList>
#include <QLabel>
#include <QSplitter>
#include <QPointer>
#include <QIcon>
#include <QShortcut>

#include <kpassworddialog.h>
#ifdef HAVE_KF5WALLET
#include <kwallet.h>
#endif

#include <KLocalizedString>

#include "ksanedevicedialog.h"
#include <ksane_debug.h>

namespace KSaneIface
{

KSaneWidget::KSaneWidget(QWidget *parent)
    : QWidget(parent), d(new KSaneWidgetPrivate(this))
{
    d->m_ksaneCoreInterface = new KSaneCore();
    
    connect(d->m_ksaneCoreInterface, &KSaneCore::scannedImageReady, d, &KSaneWidgetPrivate::imageReady);
    connect(d->m_ksaneCoreInterface, &KSaneCore::scanFinished, d, &KSaneWidgetPrivate::scanDone);
    connect(d->m_ksaneCoreInterface, &KSaneCore::userMessage, d, &KSaneWidgetPrivate::alertUser);
    connect(d->m_ksaneCoreInterface, &KSaneCore::scanProgress, d, &KSaneWidgetPrivate::updateProgress);
    connect(d->m_ksaneCoreInterface, &KSaneCore::availableDevices, d, &KSaneWidgetPrivate::signalDevListUpdate);
    connect(d->m_ksaneCoreInterface, &KSaneCore::buttonPressed, this, &KSaneWidget::buttonPressed);
    connect(d->m_ksaneCoreInterface, &KSaneCore::openedDeviceInfoUpdated, this, &KSaneWidget::openedDeviceInfoUpdated);
    
    // Create the static UI
    // create the preview
    d->m_previewViewer = new KSaneViewer(&(d->m_previewImg), this);
    connect(d->m_previewViewer, &KSaneViewer::newSelection,
            d, &KSaneWidgetPrivate::handleSelection);

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
    connect(d->m_cancelBtn, &QPushButton::clicked, this, &KSaneWidget::scanCancel);

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
    connect(d->m_zInBtn, &QToolButton::clicked, d->m_previewViewer, &KSaneViewer::zoomIn);

    d->m_zOutBtn = new QToolButton(this);
    d->m_zOutBtn->setAutoRaise(true);
    d->m_zOutBtn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    d->m_zOutBtn->setToolTip(i18n("Zoom Out"));
    connect(d->m_zOutBtn, &QToolButton::clicked, d->m_previewViewer, &KSaneViewer::zoomOut);

    d->m_zSelBtn = new QToolButton(this);
    d->m_zSelBtn->setAutoRaise(true);
    d->m_zSelBtn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-fit-best")));
    d->m_zSelBtn->setToolTip(i18n("Zoom to Selection"));
    connect(d->m_zSelBtn, &QToolButton::clicked, d->m_previewViewer, &KSaneViewer::zoomSel);

    d->m_zFitBtn = new QToolButton(this);
    d->m_zFitBtn->setAutoRaise(true);
    d->m_zFitBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-preview")));
    d->m_zFitBtn->setToolTip(i18n("Zoom to Fit"));
    connect(d->m_zFitBtn, &QToolButton::clicked, d->m_previewViewer, &KSaneViewer::zoom2Fit);

    d->m_clearSelBtn = new QToolButton(this);
    d->m_clearSelBtn->setAutoRaise(true);
    d->m_clearSelBtn->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
    d->m_clearSelBtn->setToolTip(i18n("Clear Selections"));
    connect(d->m_clearSelBtn, &QToolButton::clicked, d->m_previewViewer, &KSaneViewer::clearSelections);

    QShortcut *prevShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+P")), this);
    connect(prevShortcut, &QShortcut::activated, d, &KSaneWidgetPrivate::startPreviewScan);

    QShortcut *scanShortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+S")), this);
    connect(scanShortcut, &QShortcut::activated, d, &KSaneWidgetPrivate::startFinalScan);

    d->m_prevBtn = new QPushButton(this);
    d->m_prevBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    d->m_prevBtn->setToolTip(i18n("Scan Preview Image (%1)", prevShortcut->key().toString()));
    d->m_prevBtn->setText(i18nc("Preview button text", "Preview"));
    connect(d->m_prevBtn, &QToolButton::clicked, d, &KSaneWidgetPrivate::startPreviewScan);

    d->m_scanBtn = new QPushButton(this);
    d->m_scanBtn->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    d->m_scanBtn->setToolTip(i18n("Scan Final Image (%1)", scanShortcut->key().toString()));
    d->m_scanBtn->setText(i18nc("Final scan button text", "Scan"));
    d->m_scanBtn->setFocus(Qt::OtherFocusReason);
    connect(d->m_scanBtn, &QToolButton::clicked, d, &KSaneWidgetPrivate::startFinalScan);

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
    delete d->m_ksaneCoreInterface;
    delete d;
}

QString KSaneWidget::deviceName() const
{
    return d->m_ksaneCoreInterface->deviceName();
}

QString KSaneWidget::deviceVendor() const
{
    return d->m_ksaneCoreInterface->deviceVendor();
}

QString KSaneWidget::deviceModel() const
{
    return d->m_ksaneCoreInterface->deviceModel();
}

QString KSaneWidget::vendor() const
{
    const QString &vendor = d->m_ksaneCoreInterface->deviceVendor();
    if (!vendor.isEmpty()) {
        return vendor;
    }
    
    const QString &name = d->m_ksaneCoreInterface->deviceName();
    if (name.isEmpty()) {
        return QString();
    }
    QEventLoop loop;
    connect(this, &KSaneWidget::openedDeviceInfoUpdated, &loop, &QEventLoop::quit, Qt::QueuedConnection);
    loop.exec();
    return d->m_ksaneCoreInterface->deviceVendor();
}

QString KSaneWidget::make() const
{
    return vendor();
}

QString KSaneWidget::model() const
{
    const QString &model = d->m_ksaneCoreInterface->deviceModel();
    if (!model.isEmpty()) {
        return model;
    }
    
    const QString &name = d->m_ksaneCoreInterface->deviceName();
    if (name.isEmpty()) {
        return QString();
    }
    QEventLoop loop;
    connect(this, &KSaneWidget::openedDeviceInfoUpdated, &loop, &QEventLoop::quit, Qt::QueuedConnection);
    loop.exec();
    return d->m_ksaneCoreInterface->deviceModel();
}

QString KSaneWidget::selectDevice(QWidget *parent)
{
    QString selected_name;
    QPointer<KSaneDeviceDialog> sel = new KSaneDeviceDialog(parent);
    connect(d->m_ksaneCoreInterface, &KSaneCore::availableDevices, sel, &KSaneDeviceDialog::updateDevicesList);
    connect(sel, &KSaneDeviceDialog::requestReloadList, d->m_ksaneCoreInterface, &KSaneCore::reloadDevicesList);
    
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
    d->m_ksaneCoreInterface->reloadDevicesList();
}

bool KSaneWidget::openDevice(const QString &deviceName)
{
    KPasswordDialog               *dlg;
#ifdef HAVE_KF5WALLET
    KWallet::Wallet               *saneWallet;
#endif
    QString                        myFolderName = QStringLiteral("ksane");
    QMap<QString, QString>         wallet_entry;

    KSaneCore::KSaneOpenStatus status = d->m_ksaneCoreInterface->openDevice(deviceName);
    if (status == KSaneCore::OpeningFailed) {
        return false;
    }

    bool password_dialog_ok = true;

    // prepare wallet for authentication and create password dialog
    if (status == KSaneCore::OpeningDenied) {
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
    while (status == KSaneCore::OpeningDenied) {

        password_dialog_ok = dlg->exec();
        if (!password_dialog_ok) {
            delete dlg;
            return false; //the user canceled
        }
        
        // add/update the device user-name and password for authentication
        status = d->m_ksaneCoreInterface->openRestrictedDevice(deviceName, dlg->username(), dlg->password());

#ifdef HAVE_KF5WALLET
        // store password in wallet on successful authentication
        if (dlg->keepPassword() && status != KSaneCore::OpeningDenied) {
            QMap<QString, QString> entry;
            entry[QStringLiteral("username")] = dlg->username();
            entry[QStringLiteral("password")] = dlg->password();
            if (saneWallet) {
                saneWallet->writeMap(deviceName, entry);
            }
        }
#endif
    }

    // Create the options interface
    d->createOptInterface();

    // Enable the interface
    d->m_optsTabWidget->setDisabled(false);
    d->m_previewViewer->setDisabled(false);
    d->m_btnFrame->setDisabled(false);

    // estimate the preview size and create an empty image
    // this is done so that you can select scan area without
    // having to scan a preview.
    d->updatePreviewSize();
    QTimer::singleShot(1000, d->m_previewViewer, &KSaneViewer::zoom2Fit);
    return true;
}

bool KSaneWidget::closeDevice()
{
    bool result = d->m_ksaneCoreInterface->closeDevice();
    if (!result) {
        return false;
    }
 
    // disable the interface until a new device is opened.
    d->m_optsTabWidget->setDisabled(true);
    d->m_previewViewer->setDisabled(true);
    d->m_btnFrame->setDisabled(true);

    return true;
}

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
        d->alertUser(KSaneCore::ErrorGeneral, i18n("The image data contained 16 bits per color, "
                     "but the color depth has been truncated to 8 bits per color."));
    }
    return toQImageSilent(data, width, height, bytes_per_line, format);
}

void KSaneWidget::scanFinal()
{
    if (d->m_btnFrame->isEnabled()) {
        d->m_cancelMultiScan = false;
        d->startFinalScan();
    } else {
        // if the button frame is disabled, there is no open device to scan from
        Q_EMIT scanDone(KSaneWidget::ErrorGeneral, QString());
    }
}

void KSaneWidget::startPreviewScan()
{
    if (d->m_btnFrame->isEnabled()) {
        d->m_cancelMultiScan = false;
        d->startPreviewScan();
    } else {
        // if the button frame is disabled, there is no open device to scan from
        Q_EMIT scanDone(KSaneWidget::ErrorGeneral, QString());
    }
}

void KSaneWidget::scanCancel()
{
    d->m_cancelMultiScan = true;
    d->m_ksaneCoreInterface->stopScan();
}

void KSaneWidget::setPreviewResolution(float dpi)
{
    d->m_previewDPI = dpi;
}

void KSaneWidget::getOptVals(QMap <QString, QString> &opts)
{
    opts.clear();
    opts = d->m_ksaneCoreInterface->getOptionsMap();
}

bool KSaneWidget::getOptVal(const QString &optname, QString &value)
{
    const auto optionsMap = d->m_ksaneCoreInterface->getOptionsMap();
    auto it = optionsMap.constBegin();
    while (it != optionsMap.constEnd()) {
        if(it.key() == optname) {
            value = it.value();
            return !value.isEmpty();
        }
        it++;        
    }
    return false;
}

int KSaneWidget::setOptVals(const QMap <QString, QString> &opts)
{
    int ret = 0;

    ret = d->m_ksaneCoreInterface->setOptionsMap(opts);
    
    if ((d->m_splitGamChB) &&
            (d->m_optGamR) &&
            (d->m_optGamG) &&
            (d->m_optGamB)) {
        // check if the current gamma values are identical. if they are identical,
        // uncheck the "Separate color intensity tables" checkbox
        QVariant redGamma = d->m_optGamR->value();
        QVariant greenGamma = d->m_optGamG->value();
        QVariant blueGamma = d->m_optGamB->value();

        if ((redGamma == greenGamma) && (greenGamma == blueGamma)) {
            d->m_splitGamChB->setChecked(false);
            // set the values to the common gamma widget
            d->m_commonGamma->setValues(redGamma);
        } else {
            d->m_splitGamChB->setChecked(true);
        }
    }
    return ret;
}

bool KSaneWidget::setOptVal(const QString &option, const QString &value)
{
    if (d->m_scanOngoing) {
        return false;
    }

    const auto optionsList = d->m_ksaneCoreInterface->getOptionsList();
    for (auto &writeOption : optionsList) {
        if (writeOption->name() == option) {
            if (writeOption->setValue(value)) {
                if ((d->m_splitGamChB) &&
                        (d->m_optGamR) &&
                        (d->m_optGamG) &&
                        (d->m_optGamB) &&
                        ((writeOption == d->m_optGamR) ||
                        (writeOption == d->m_optGamG) ||
                        (writeOption == d->m_optGamB))) {
                    // check if the current gamma values are identical. if they are identical,
                    // uncheck the "Separate color intensity tables" checkbox
                    QVariant redGamma = d->m_optGamR->value();
                    QVariant greenGamma = d->m_optGamG->value();
                    QVariant blueGamma = d->m_optGamB->value();
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
        QVariant value = d->m_optRes->value();
        if (!value.isNull()) {
            return value.toFloat();
        }
    }
    return 0.0; // Failure to read DPI
}

float KSaneWidget::scanAreaWidth()
{
    float result = 0.0;
    if (d->m_optBrX) {
        if (d->m_optBrX->valueUnit() == KSaneOption::UnitPixel) {
            result = d->m_optBrX->maximumValue().toFloat();
            float dpi = currentDPI();
            if (dpi < 1) {
                qCDebug(KSANE_LOG) << "Broken DPI value";
                dpi = 1.0;
            }
            result = result / dpi / 25.4;
        } else if (d->m_optBrX->valueUnit() == KSaneOption::UnitMilliMeter) {
            result = d->m_optBrX->maximumValue().toFloat();
        }
    }
    return result;
}

float KSaneWidget::scanAreaHeight()
{
    float result = 0.0;
    if (d->m_optBrY) {
        if (d->m_optBrY->valueUnit() == KSaneOption::UnitPixel) {
            result = d->m_optBrY->maximumValue().toFloat();
            float dpi = currentDPI();
            if (dpi < 1) {
                qCDebug(KSANE_LOG) << "Broken DPI value";
                dpi = 1.0;
            }
            result = result / dpi / 25.4;
        } else if (d->m_optBrY->valueUnit() == KSaneOption::UnitMilliMeter) {
            result = d->m_optBrY->maximumValue().toFloat();
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
        QTimer::singleShot(0, d->m_optionsCollapser, &SplitterCollapser::slotCollapse);
    } else {
        QTimer::singleShot(0, d->m_optionsCollapser, &SplitterCollapser::slotRestore);
    }
}

void KSaneWidget::setScanButtonHidden(bool hidden)
{
    d->m_scanBtn->setHidden(hidden);
}

}  // NameSpace KSaneIface
