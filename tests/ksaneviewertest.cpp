/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2010-03-17
* Description : Preview image viewer that can handle a selection.
*
* SPDX-FileCopyrightText: 2010 Kare Sars <kare dot sars at iki dot fi>
* SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#include "ksaneviewer.h"

#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (argc != 2) {
        qDebug() << "An image filename is needed.";
        return 1;
    }
    QImage img(QString::fromUtf8(argv[1]));

    KSaneIface::KSaneViewer viewer(&img);

    viewer.findSelections();

    viewer.show();

    viewer.zoom2Fit();

    return app.exec();
}
