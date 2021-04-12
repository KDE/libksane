/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2018 Alexander Volkov <a.volkov@rusbitech.ru>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_PREVIEW_IMAGE_BUILDER_H
#define KSANE_PREVIEW_IMAGE_BUILDER_H

extern "C"
{
#include <sane/sane.h>
}

class QImage;

namespace KSaneIface
{
class KSanePreviewImageBuilder
{
public:
    KSanePreviewImageBuilder(QImage *img);

    void start(const SANE_Parameters &params);
    void beginFrame(const SANE_Parameters &params);
    bool copyToImage(const SANE_Byte readData[], int read_bytes);
    bool imageResized();

private:
    void renewImage();

    SANE_Parameters m_params;
    int m_frameRead;
    int m_pixel_x;
    int m_pixel_y;
    int m_px_colors[3];
    int m_px_c_index;

    QImage *m_img;

    bool m_imageResized;
};
}

#endif // KSANE_PREVIEW_IMAGE_BUILDER_H
