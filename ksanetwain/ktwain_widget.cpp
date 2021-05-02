/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-05-11
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009, 2017 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksanewidget.h"
#include "ksaneoption.h"

// Qt includes
#include <QMap>
#include <QHBoxLayout>
#include <QTimer>
#include <QDebug>

// KDE includes
#include <KLocalizedString>
#include <QPushButton>

// Local includes
#include "twainiface.h"

namespace KSaneIface
{

KSaneWidget::KSaneWidget(QWidget *parent)
    : QWidget(parent), d(new KSaneWidgetPrivate)
{
    QPushButton *openDialog = new QPushButton;
    openDialog->setText(i18n("Open scan dialog"));
    QHBoxLayout *lay = new QHBoxLayout;
    lay->addWidget(openDialog);
    setLayout(lay);

    connect(openDialog, &QPushButton::clicked, d, &KSaneWidgetPrivate::ReOpenDialog);

    // Forward signals from the private class
    //connect(d, SIGNAL(scanProgress(int)), this, SIGNAL(scanProgress(int)));
    connect(d, &KSaneWidgetPrivate::ImageReady, this, &KSaneWidget::imageReady);
    connect(d, &KSaneWidgetPrivate::qImageReady, this, &KSaneWidget::scannedImageReady);
    //connect(d, SIGNAL(scanDone(int,QString)), this, SIGNAL(scanDone(int,QString)));

}

KSaneWidget::~KSaneWidget()
{
    delete d;
}

QString KSaneWidget::selectDevice(QWidget *parent)
{
    //qDebug() << "KSaneWidget::selectDevice";
    return d->SelectSource();
}

bool KSaneWidget::openDevice(const QString &device_name)
{
    //qDebug() << "KSaneWidget::openDevice" << device_name;
    if (!d->OpenSource(device_name)) {
        return false;
    }
    QTimer::singleShot(0, d, &KSaneWidgetPrivate::OpenDialog);

    return true;
}

bool KSaneWidget::closeDevice()
{
    //qDebug() << "KSaneWidget::closeDevice";
    d->CloseDSM();
    return true;
}

QImage KSaneWidget::toQImage(const QByteArray &data,
                             int width,
                             int height,
                             int bytes_per_line,
                             ImageFormat format)
{
    return toQImageSilent(data,
                          width,
                          height,
                          bytes_per_line,
                          format);
}

QImage KSaneWidget::toQImageSilent(const QByteArray &data,
                                   int width,
                                   int height,
                                   int bytes_per_line,
                                   ImageFormat format)
{
    return KSaneWidget::toQImageSilent(data,
                                       width,
                                       height,
                                       bytes_per_line,
                                       (int) currentDPI(),
                                       format);
}

QImage KSaneWidget::toQImageSilent(const QByteArray &data,
                                   int width,
                                   int height,
                                   int bytes_per_line,
                                   int dpi,
                                   ImageFormat format)
{
    QImage img = QImage::fromData(data, "BMP");
    return img;
}

void KSaneWidget::scanFinal()
{
}

void KSaneWidget::startPreviewScan()
{
}

void KSaneWidget::scanCancel()
{
    Q_EMIT scanProgress(0);
}

// stubbed functions that will not get an implementation any time soon.
QString KSaneWidget::deviceName() const
{
    return QString();
}
QString KSaneWidget::deviceVendor() const
{
    return QString();
}
QString KSaneWidget::deviceModel() const
{
    return QString();
}
QString KSaneWidget::vendor() const
{
    return QString();
}
QString KSaneWidget::make() const
{
    return QString();
}
QString KSaneWidget::model() const
{
    return QString();
}
void KSaneWidget::getOptVals(QMap <QString, QString> &opts)
{
    opts.clear();
}
bool KSaneWidget::getOptVal(const QString &optname, QString &value)
{
    return false;
}
int KSaneWidget::setOptVals(const QMap <QString, QString> &opts)
{
    return 0;
}
bool KSaneWidget::setOptVal(const QString &option, const QString &value)
{
    return false;
}
void KSaneWidget::setScanButtonText(const QString &scanLabel) {}
void KSaneWidget::setPreviewButtonText(const QString &previewLabel) {}
void KSaneWidget::enableAutoSelect(bool enable) {}
float KSaneWidget::currentDPI()
{
    return 0.0;
}
void KSaneWidget::setPreviewResolution(float dpi)
{
    Q_UNUSED(dpi);
}
void KSaneWidget::setSelection(QPointF lefttop, QPointF rightbottom)
{
    Q_UNUSED(lefttop);
    Q_UNUSED(rightbottom);
}
void KSaneWidget::setOptionsCollapsed(bool) {}
void KSaneWidget::setScanButtonHidden(bool) {}
void KSaneWidget::initGetDeviceList() const {}
QList<KSaneOption *> KSaneWidget::getOptionsList()
{
    return QList<KSaneOption *>();
}
KSaneOption *KSaneWidget::getOption(KSaneOptionName optionEnum)
{
    return nullptr;
}
KSaneOption *KSaneWidget::getOption(QString optionName)
{
    return nullptr;
}
}  // NameSpace KSaneIface
