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

#include "ksanepreviewimagebuilder.h"

#include <QDebug>
#include <QImage>

namespace KSaneIface
{
KSanePreviewImageBuilder::KSanePreviewImageBuilder(QImage *img)
    : m_frameRead(0),
      m_pixel_x(0),
      m_pixel_y(0),
      m_px_c_index(0),
      m_img(img),
      m_imageResized(false)
{
    m_px_colors[0] = 0;
    m_px_colors[1] = 0;
    m_px_colors[2] = 0;
}

void KSanePreviewImageBuilder::start(const SANE_Parameters &params)
{
    beginFrame(params);

    // create a new image if necessary
    if ((m_img->height() != m_params.lines) ||
            (m_img->width()  != m_params.pixels_per_line)) {
        // just hope that the frame size is not changed between different frames of the same image.
        if (m_params.lines > 0) {
            *m_img = QImage(m_params.pixels_per_line, m_params.lines, QImage::Format_RGB32);
        } else {
            // handscanners have the number of lines -1 -> make room for something
            *m_img = QImage(m_params.pixels_per_line, m_params.pixels_per_line, QImage::Format_RGB32);
        }
        m_img->fill(0xFFFFFFFF);
    }
    m_imageResized = false;
}

void KSanePreviewImageBuilder::beginFrame(const SANE_Parameters &params)
{
    m_params = params;
    m_frameRead  = 0;
    m_pixel_x    = 0;
    m_pixel_y    = 0;
    m_px_c_index = 0;
}

#define inc_pixel(x,y,ppl) { x++; if (x>=ppl) { y++; x=0;} }
#define inc_color_index(index) { index++; if (index==3) index=0;}

#define index_red8_to_argb8(i)   (i*4 + 2)
#define index_red16_to_argb8(i)  (i*2 + 2)

#define index_green8_to_argb8(i)  (i*4 + 1)
#define index_green16_to_argb8(i) (i*2 + 1)

#define index_blue8_to_argb8(i)  (i*4)
#define index_blue16_to_argb8(i) (i*2)

bool KSanePreviewImageBuilder::copyToImage(const SANE_Byte readData[], int read_bytes)
{
    int index;
    uchar *imgBits = m_img->bits();
    switch (m_params.format) {
    case SANE_FRAME_GRAY:
        if (m_params.depth == 1) {
            int i, j;
            for (i = 0; i < read_bytes; i++) {
                if (m_pixel_y >= m_img->height()) {
                    renewImage();
                }
                for (j = 7; j >= 0; --j) {
                    if ((readData[i] & (1 << j)) == 0) {
                        m_img->setPixel(m_pixel_x,
                                        m_pixel_y,
                                        qRgb(255, 255, 255));
                    } else {
                        m_img->setPixel(m_pixel_x,
                                        m_pixel_y,
                                        qRgb(0, 0, 0));
                    }
                    m_pixel_x++;
                    if (m_pixel_x >= m_params.pixels_per_line) {
                        m_pixel_x = 0;
                        m_pixel_y++;
                        break;
                    }
                    if (m_pixel_y >= m_params.lines) {
                        break;
                    }
                }
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                index = m_frameRead * 4;
                if ((index + 2) > m_img->byteCount()) {
                    renewImage();
                    imgBits = m_img->bits();
                }
                imgBits[index    ] = readData[i];
                imgBits[index + 1] = readData[i];
                imgBits[index + 2] = readData[i];
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                if (m_frameRead % 2 == 0) {
                    index = m_frameRead * 2;
                    if ((index + 2) > m_img->byteCount()) {
                        renewImage();
                        imgBits = m_img->bits();
                    }
                    imgBits[index    ] = readData[i + 1];
                    imgBits[index + 1] = readData[i + 1];
                    imgBits[index + 2] = readData[i + 1];
                }
                m_frameRead++;
            }
            return true;
        }
        break;

    case SANE_FRAME_RGB:
        if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                m_px_colors[m_px_c_index] = readData[i];
                inc_color_index(m_px_c_index);
                m_frameRead++;
                if (m_px_c_index == 0) {
                    if (m_pixel_y >= m_img->height()) {
                        renewImage();
                    }
                    m_img->setPixel(m_pixel_x,
                                    m_pixel_y,
                                    qRgb(m_px_colors[0],
                                         m_px_colors[1],
                                         m_px_colors[2]));
                    inc_pixel(m_pixel_x, m_pixel_y, m_params.pixels_per_line);
                }
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                m_frameRead++;
                if (m_frameRead % 2 == 0) {
                    m_px_colors[m_px_c_index] = readData[i];
                    inc_color_index(m_px_c_index);
                    if (m_px_c_index == 0) {
                        if (m_pixel_y >= m_img->height()) {
                            renewImage();
                        }
                        m_img->setPixel(m_pixel_x,
                                        m_pixel_y,
                                        qRgb(m_px_colors[0],
                                             m_px_colors[1],
                                             m_px_colors[2]));
                        inc_pixel(m_pixel_x, m_pixel_y, m_params.pixels_per_line);
                    }
                }
            }
            return true;
        }
        break;

    case SANE_FRAME_RED:
        if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                if (index_red8_to_argb8(m_frameRead) > m_img->byteCount()) {
                    renewImage();
                    imgBits = m_img->bits();
                }
                imgBits[index_red8_to_argb8(m_frameRead)] = readData[i];
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                if (m_frameRead % 2 == 0) {
                    if (index_red16_to_argb8(m_frameRead) > m_img->byteCount()) {
                        renewImage();
                        imgBits = m_img->bits();
                    }
                    imgBits[index_red16_to_argb8(m_frameRead)] = readData[i + 1];
                }
                m_frameRead++;
            }
            return true;
        }
        break;

    case SANE_FRAME_GREEN:
        if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                if (index_green8_to_argb8(m_frameRead) > m_img->byteCount()) {
                    renewImage();
                    imgBits = m_img->bits();
                }
                imgBits[index_green8_to_argb8(m_frameRead)] = readData[i];
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                if (m_frameRead % 2 == 0) {
                    if (index_green16_to_argb8(m_frameRead) > m_img->byteCount()) {
                        renewImage();
                        imgBits = m_img->bits();
                    }
                    imgBits[index_green16_to_argb8(m_frameRead)] = readData[i + 1];
                }
                m_frameRead++;
            }
            return true;
        }
        break;

    case SANE_FRAME_BLUE:
        if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                if (index_blue8_to_argb8(m_frameRead) > m_img->byteCount()) {
                    renewImage();
                    imgBits = m_img->bits();
                }
                imgBits[index_blue8_to_argb8(m_frameRead)] = readData[i];
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                if (m_frameRead % 2 == 0) {
                    if (index_blue16_to_argb8(m_frameRead) > m_img->byteCount()) {
                        renewImage();
                        imgBits = m_img->bits();
                    }
                    imgBits[index_blue16_to_argb8(m_frameRead)] = readData[i + 1];
                }
                m_frameRead++;
            }
            return true;
        }
        break;
    }

    qWarning() << "Format" << m_params.format
               << "and depth" << m_params.depth
               << "is not yet supported by libksane!";
    return false;
}

bool KSanePreviewImageBuilder::imageResized()
{
    if (m_imageResized) {
        m_imageResized = false;
        return true;
    }
    return false;
}

void KSanePreviewImageBuilder::renewImage()
{
    // resize the image
    *m_img = m_img->copy(0, 0, m_img->width(), m_img->height() + m_img->width());
    m_imageResized = true;
}
} // NameSpace KSaneIface
