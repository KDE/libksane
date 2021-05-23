/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2018 Alexander Volkov <a.volkov@rusbitech.ru>
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_IMAGE_BUILDER_H
#define KSANE_IMAGE_BUILDER_H

extern "C"
{
#include <sane/sane.h>
}

class QImage;

namespace KSaneIface
{

/* Constructs a QImage out of the raw scanned data retrieved via libsane */
class KSaneImageBuilder
{
public:
    KSaneImageBuilder(QImage *image, int *dpi);

    void start(const SANE_Parameters &params);
    void beginFrame(const SANE_Parameters &params);
    bool copyToImage(const SANE_Byte readData[], int read_bytes);
    void setDPI(int dpi);

private:
    void renewImage();
    void incrementPixelData();

    SANE_Parameters m_params;
    int m_frameRead = 0;
    int m_pixelX = 0;
    int m_pixelY = 0;
    int m_pixelData[6];
    int m_pixelDataIndex = 0;

    QImage *m_image;
    int *m_dpi;
};

}

#endif // KSANE_IMAGE_BUILDER_H
