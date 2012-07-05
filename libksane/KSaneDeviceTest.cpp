/* ============================================================
*
* This file is part of the KDE project
*
* Date        : 2010-03-17
* Description : Preview image viewer that can handle a selection.
*
* Copyright (C) 2010 by Kare Sars <kare dot sars at iki dot fi>
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

#include "KSaneViewer.h"
#include "KSaneDevice.h"

#include <KDebug>
#include <QTimer>
#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>

int main (int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (argc != 2) {
        kDebug() << "A device is needed.";
        return 1;
    }

    KSaneDevice device;
    if (!device.openDevice(argv[1])) {
        kDebug() << "Failed to open device:" << argv[1];
        return 1;
    }

    KSaneViewer *viewer= new KSaneViewer(device.previewImage());

    //device.setPreviewResolution(200);

    QObject::connect(&device, SIGNAL(previewImageResized()), viewer, SLOT(imageResized()));
    QObject::connect(&device, SIGNAL(previewProgress(int)), viewer, SLOT(imageUpdated()));

    QDialog dialog;
    QHBoxLayout *layout = new QHBoxLayout(&dialog);
    layout->addWidget(viewer);

    QTimer::singleShot(2000, &device, SLOT(scanPreview()));
    return dialog.exec();
}
