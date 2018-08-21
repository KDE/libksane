/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2009 by Kare Sars <kare dot sars at iki dot fi>
 * Copyright (C) 2018 by Alexander Volkov <a.volkov@rusbitech.ru>
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
