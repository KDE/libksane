/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2009 by Kare Sars <kare dot sars at iki dot fi>
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

#ifndef KSANE_WIDGET_PRIVATE_H
#define KSANE_WIDGET_PRIVATE_H

// Sane includes.
extern "C"
{
    #include <sane/saneopts.h>
    #include <sane/sane.h>
}

// Qt includes.
#include <QtGui/QWidget>
#include <QCheckBox>
#include <QTimer>
#include <QTime>
#include <QProgressBar>
#include <QThread>

// KDE includes
#include <KTabWidget>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KPushButton>

// Local includes
#include "ksane.h"
#include "ksane_option.h"
#include "ksane_viewer.h"
#include "labeled_separator.h"
#include "labeled_gamma.h"

#define inc_pixel(x,y,ppl) { x++; if (x>=ppl) { y++; x=0;} }

#define IMG_DATA_R_SIZE 100000

/** This namespace collects all methods and classes in LibKSane. */
namespace KSaneIface
{
    class KSaneReadThread: public QThread
    {
        public:
            KSaneReadThread(SANE_Handle handle, SANE_Byte *data, SANE_Int maxBytes);
            void run();
            SANE_Status    status;
            SANE_Int       readBytes;
        private:
            SANE_Byte     *m_data;
            const SANE_Int m_maxBytes;
            SANE_Handle    m_saneHandle;
    };

    class KSaneWidgetPrivate: public QObject
    {
        Q_OBJECT

        typedef enum
        {
            READ_ON_GOING,
            READ_ERROR,
            READ_CANCEL,
            READ_FINISHED,
            READ_READY_SEL,
            READ_READY
        } ReadStatus;
        
        public:
            KSaneWidgetPrivate();
            void clearDeviceOptions();
            void createOptInterface();
            void updatePreviewSize();
            void copyToScanData(int read_bytes);
            void copyToPreview(int read_bytes);
            void setDefaultValues();
            void setBusy(bool busy);
            void scanDone();
            void scanCancel();
            KSaneOption *getOption(const QString &name);
            KSaneWidget::ImageFormat getImgFormat(SANE_Parameters &params);
            int getBytesPerLines(SANE_Parameters &params);
            
        Q_SIGNALS:
            void imageReady(QByteArray &data, int width, int height, int bytes_per_line, int format);
            
            void scanProgress(int percent);
            void scanDone(int status, const QString &errStr);
            
        public Q_SLOTS:
            void scanFinal();
            
        private Q_SLOTS:
            void scheduleValReload();
            void optReload();
            void valReload();
            void handleSelection(float tl_x, float tl_y, float br_x, float br_y);
            void scanPreview();
            void setTLX(float x);
            void setTLY(float y);
            void setBRX(float x);
            void setBRY(float y);
            void startScan();
            void processData();
            
        public:
            // backend independent
            KTabWidget         *m_optsTabWidget;
            QScrollArea        *m_basicScrollA;
            QWidget            *m_basicOptsTab;
            QWidget            *m_colorOpts;
            QScrollArea        *m_otherScrollA;
            QWidget            *m_otherOptsTab;
            
            KSaneViewer        *m_previewViewer;
            KPushButton        *m_scanBtn;
            KPushButton        *m_prevBtn;
            KPushButton        *m_zInBtn;
            KPushButton        *m_zOutBtn;
            KPushButton        *m_zSelBtn;
            KPushButton        *m_zFitBtn;
            KPushButton        *m_cancelBtn;
            QLabel             *m_warmingUp;
            QProgressBar       *m_progressBar;
            
            // device info
            SANE_Handle         m_saneHandle;
            QString             m_modelName;
            QString             m_vendor;
            QString             m_model;
            
            // Option variables
            QList<KSaneOption*> m_optList;
            KSaneOption        *m_optSource;
            KSaneOption        *m_optMode;
            KSaneOption        *m_optDepth;
            KSaneOption        *m_optRes;
            KSaneOption        *m_optResY;
            KSaneOption        *m_optTlX;
            KSaneOption        *m_optTlY;
            KSaneOption        *m_optBrX;
            KSaneOption        *m_optBrY;
            KSaneOption        *m_optGamR;
            KSaneOption        *m_optGamG;
            KSaneOption        *m_optGamB;
            QCheckBox          *m_splitGamChB;
            LabeledGamma       *m_commonGamma;
            KSaneOption        *m_optPreview;
            
            // preview variables
            float               m_previewWidth;
            float               m_previewHeight;
            float               m_previewDPI;
            QImage              m_previewImg;
            bool                m_autoSelect;
            int                 m_selIndex;
            
            // final image data
            QByteArray          m_scanData;
            
            // option handling
            QTimer              m_readValsTmr;
            QTimer              m_startScanTmr;
            KSaneReadThread    *m_readThread;
            QTime               m_timeSinceUpd;
            
            // general scanning
            bool                m_isPreview;
            ReadStatus          m_readStatus;
            SANE_Parameters     m_params;
            SANE_Byte           m_saneReadBuffer[IMG_DATA_R_SIZE];
            int                 m_frameSize;
            int                 m_frameRead;
            int                 m_dataSize;
            int                 m_frame_t_count;
            int                 m_pixel_x;
            int                 m_pixel_y;
            int                 m_px_colors[3];
            int                 m_px_c_index;
    };


}  // NameSpace KSaneIface

#endif // SANE_WIDGET_H
