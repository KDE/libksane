/* ============================================================
*
* This file is part of the KDE project
*
* Copyright (C) 2010-2012 by Kare Sars <kare.sars@iki.fi>
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
#include <QLabel>
#include <QTime>

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

    QList<KSaneOption*> options = device.options();

    KSaneOption *test = device.option("enable-test-options");
    test->setValue(true);

    QDialog optdialog;
    QVBoxLayout *optlayout = new QVBoxLayout(&optdialog);
    QLabel *label;
    for (int i=0; i<options.size(); i++) {
        kDebug() << options[i]->saneName() << options[i]->title() << options[i]->value() << options[i]->visibility();
        switch (options[i]->type())
        {
            case KSaneOption::Type_None:
                label = 0;
                break;
            case KSaneOption::Type_CheckBox:
                label = new QLabel("CheckBox " + options[i]->title() + options[i]->value().toString());
                break;
            case KSaneOption::Type_Slider:
                label = new QLabel("Slider   " + options[i]->title() + options[i]->value().toString());
                break;
            case KSaneOption::Type_SliderF:
                label = new QLabel("SliderF  " + options[i]->title() + options[i]->value().toString());
                break;
            case KSaneOption::Type_Combo:
                label = new QLabel("Combo    " + options[i]->title() + options[i]->value().toString() + options[i]->valueList().join(","));
                break;
            case KSaneOption::Type_Entry:
                label = new QLabel("Entry    " + options[i]->title() + options[i]->value().toString());
                break;
            case KSaneOption::Type_Gamma:
                label = new QLabel("Gamma    " + options[i]->title() + options[i]->value().toString());
                break;
            case KSaneOption::Type_Button:
                label = new QLabel("Button   " + options[i]->title() + options[i]->value().toString());
                break;
        }

        if (label) {
            optlayout->addWidget(label);
            label->setVisible(options[i]->visibility() != KSaneOption::Hidden);
        }
    }
    optdialog.show();
    dialog.show();
    QTimer::singleShot(2000, &device, SLOT(scanPreview()));
    return dialog.exec();
}
