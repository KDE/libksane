/* ============================================================
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2018 Alexander Volkov <a.volkov@rusbitech.ru>
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneimagebuilder.h"

#include <QImage>

#include <ksane_debug.h>

namespace KSaneIface
{
KSaneImageBuilder::KSaneImageBuilder(QImage *image, int *dpi)
    : m_image(image), m_dpi(dpi)
{
    m_pixelData[0] = 0;
    m_pixelData[1] = 0;
    m_pixelData[2] = 0;
    m_pixelData[3] = 0;
    m_pixelData[4] = 0;
    m_pixelData[5] = 0;
}

void KSaneImageBuilder::start(const SANE_Parameters &params)
{
    beginFrame(params);
    QImage::Format imageFormat = QImage::Format_RGB32;
    if (m_params.format == SANE_FRAME_GRAY) {
        switch (m_params.depth) {
            case 1:
                imageFormat = QImage::Format_Mono;
                break;
            case 16:
                imageFormat = QImage::Format_Grayscale16;
                break;
            default:
                imageFormat = QImage::Format_Grayscale8;
                break;
        }
    } else if (m_params.depth > 8) {
        imageFormat = QImage::Format_RGBX64;
    }
    // create a new image if necessary
    if ((m_image->height() != m_params.lines) ||
            (m_image->width() != m_params.pixels_per_line) || m_image->format() != imageFormat) {
        // just hope that the frame size is not changed between different frames of the same image.

        int pixelLines = m_params.lines;
        // handscanners have the number of lines -1 -> make room for something
        if (m_params.lines <= 0) {
            pixelLines = m_params.pixels_per_line;
        }
        *m_image = QImage(m_params.pixels_per_line, pixelLines, imageFormat);
        m_image->fill(0xFFFFFFFF);
        if (m_image->format() == QImage::Format_Mono) {
            m_image->setColorTable(QVector<QRgb>({0xFFFFFFFF,0xFF000000}));
        }
        int dpm = *m_dpi * (1000.0 / 25.4);
        m_image->setDotsPerMeterX(dpm);
        m_image->setDotsPerMeterY(dpm);
    }
    m_imageResized = false;
}

void KSaneImageBuilder::beginFrame(const SANE_Parameters &params)
{
    m_params = params;
    m_frameRead  = 0;
    m_pixelX    = 0;
    m_pixelY    = 0;
    m_pixelDataIndex = 0;
}

bool KSaneImageBuilder::copyToImage(const SANE_Byte readData[], int read_bytes)
{
    switch (m_params.format) {
    case SANE_FRAME_GRAY:
        if (m_params.depth == 1) {
            for (int i = 0; i < read_bytes; i++) {
                if (m_pixelY >= m_image->height()) {
                    renewImage();
                }
                uchar *imageBits = m_image->scanLine(m_pixelY);
                imageBits[m_pixelX / 8] = readData[i];
                m_pixelX += 8;
                if (m_pixelX >= m_params.pixels_per_line) {
                    m_pixelX = 0;
                    m_pixelY++;
                }
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                uchar *grayScale = m_image->scanLine(m_pixelY);
                grayScale[m_pixelX] = readData[i];
                incrementPixelData();
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                m_pixelData[m_pixelDataIndex] = readData[i];
                m_pixelDataIndex++;
                if (m_pixelDataIndex == 2) {
                    m_pixelDataIndex = 0;
                }
                if (m_pixelDataIndex == 0) {
                    if (m_pixelY >= m_image->height()) {
                        renewImage();
                    }
                    quint16 *grayScale = reinterpret_cast<quint16*>(m_image->scanLine(m_pixelY));
                    grayScale[m_pixelX] = m_pixelData[0] + (m_pixelData[1] << 8);
                    incrementPixelData();
                }
                m_frameRead++;
            }
            return true;
        }
        break;

    case SANE_FRAME_RGB:
        if (m_params.depth == 8) {
            for (int i = 0; i < read_bytes; i++) {
                m_pixelData[m_pixelDataIndex] = readData[i];
                m_pixelDataIndex++;
                if (m_pixelDataIndex == 3) {
                    m_pixelDataIndex = 0;
                }
                if (m_pixelDataIndex == 0) {
                    if (m_pixelY >= m_image->height()) {
                        renewImage();
                    }
                    QRgb *rgbData = reinterpret_cast<QRgb*>(m_image->scanLine(m_pixelY));
                    rgbData[m_pixelX] = qRgb(m_pixelData[0], m_pixelData[1], m_pixelData[2]);
                    incrementPixelData();
                }
                m_frameRead++;
            }
            return true;
        } else if (m_params.depth == 16) {
            for (int i = 0; i < read_bytes; i++) {
                m_pixelData[m_pixelDataIndex] = readData[i];
                m_pixelDataIndex++;
                if (m_pixelDataIndex == 6) {
                    m_pixelDataIndex = 0;
                }
                if (m_pixelDataIndex == 0) {
                    if (m_pixelY >= m_image->height()) {
                        renewImage();
                    }
                    QRgba64 *rgbData = reinterpret_cast<QRgba64*>(m_image->scanLine(m_pixelY));
                    rgbData[m_pixelX] = QRgba64::fromRgba64((m_pixelData[0] + (m_pixelData[1] << 8)),
                        (m_pixelData[2] + (m_pixelData[3] << 8)), (m_pixelData[4] + (m_pixelData[5] << 8)), 0xFFFF);
                    incrementPixelData();
                }
            }
            m_frameRead++;
            return true;
        }
        break;

    case SANE_FRAME_RED: {
            uchar *imgBits = m_image->bits();
            int index = 0;
            if (m_params.depth == 8) {
                for (int i = 0; i < read_bytes; i++) {
                    index = m_frameRead * 4 + 2;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            } else if (m_params.depth == 16) {
                for (int i = 0; i < read_bytes; i++) {
                    index = (m_frameRead - m_frameRead % 2) * 4 + m_frameRead % 2;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            }
            break;
        }
    case SANE_FRAME_GREEN: {
            uchar *imgBits = m_image->bits();
            int index = 0;
            if (m_params.depth == 8) {
                for (int i = 0; i < read_bytes; i++) {
                    int index = m_frameRead * 4 + 1;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            } else if (m_params.depth == 16) {
                for (int i = 0; i < read_bytes; i++) {
                    index = (m_frameRead - m_frameRead % 2) * 4 + 2 + m_frameRead % 2;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            }
            break;
        }
    case SANE_FRAME_BLUE: {
            uchar *imgBits = m_image->bits();
            int index = 0;
            if (m_params.depth == 8) {
                for (int i = 0; i < read_bytes; i++) {
                    int index = m_frameRead * 4;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            } else if (m_params.depth == 16) {
                for (int i = 0; i < read_bytes; i++) {
                    index = (m_frameRead - m_frameRead % 2) * 4 + 4 + m_frameRead % 2;
                    if (index > m_image->sizeInBytes()) {
                        renewImage();
                    }
                    imgBits[index] = readData[i];
                    m_frameRead++;
                }
                return true;
            }
            break;
        }
    }

    qCWarning(KSANE_LOG) << "Format" << m_params.format
               << "and depth" << m_params.depth
               << "is not yet supported by libksane!";
    return false;
}

bool KSaneImageBuilder::imageResized()
{
    if (m_imageResized) {
        m_imageResized = false;
        return true;
    }
    return false;
}

void KSaneImageBuilder::renewImage()
{
    // resize the image
    *m_image = m_image->copy(0, 0, m_image->width(), m_image->height() + m_image->width());
    m_imageResized = true;
}

void KSaneImageBuilder::incrementPixelData()
{
    m_pixelX++;
    if (m_pixelX >= m_params.pixels_per_line) {
        m_pixelY++;
        m_pixelX=0;
    }
}

} // NameSpace KSaneIface
