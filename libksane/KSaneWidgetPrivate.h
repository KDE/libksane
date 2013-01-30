/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Copyright (C) 2009-2012 by Kare Sars <kare.sars@iki.fi>
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

#ifndef KSaneWidgetPrivate_h
#define KSaneWidgetPrivate_h

// Sane includes
extern "C"
{
    #include <sane/saneopts.h>
    #include <sane/sane.h>
}

// Qt includes
#include <QtGui/QWidget>
#include <QCheckBox>
#include <QTimer>
#include <QTime>
#include <QProgressBar>

// KDE includes
#include <KTabWidget>
#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <KPushButton>
#include <KToolBar>

// Local includes
#include "ksane.h"
#include "KSaneOptInternal.h"
#include "KSaneViewer.h"
#include "KSaneGamma.h"
#include "KSaneCheckBox.h"
#include "splittercollapser.h"
#include "KSaneScanThread.h"
#include "KSanePreviewThread.h"
#include "KSaneFindDevicesThread.h"

#define IMG_DATA_R_SIZE 100000

/** This namespace collects all methods and classes in LibKSane. */
    class KSaneWidgetPrivate: public QObject
    {
        Q_OBJECT

        public:
            KSaneWidgetPrivate(KSaneWidget *);
            void clearDeviceOptions();
            void createOptInterface();
            void updatePreviewSize();
            void setDefaultValues();
            void setBusy(bool busy);
            KSaneOptInternal *getOption(const QString &name);
            KSaneWidget::ImageFormat getImgFormat(SANE_Parameters &params);
            int getBytesPerLines(SANE_Parameters &params);

        public Q_SLOTS:
            void devListUpdated();
            void signalDevListUpdate();
            void startFinalScan();
            void previewScanDone();
            void oneFinalScanDone();
            void updateProgress();

        private Q_SLOTS:
            void scheduleValReload();
            void optReload();
            void valReload();
            void handleSelection(qreal tl_x, qreal tl_y, qreal br_x, qreal br_y);
            void setTLX(qreal x);
            void setTLY(qreal y);
            void setBRX(qreal x);
            void setBRY(qreal y);

            void startPreviewScan();

            void checkInvert();
            void invertPreview();
            void pollPollOptions();

        public:
            void alertUser(int type, const QString &strStatus);

        public:
            // backend independent
            KTabWidget         *m_optsTabWidget;
            QScrollArea        *m_basicScrollA;
            QWidget            *m_basicOptsTab;
            QWidget            *m_colorOpts;
            QScrollArea        *m_otherScrollA;
            QWidget            *m_otherOptsTab;
            KSaneCheckBox    *m_invertColors;

            QSplitter          *m_splitter;
            SplitterCollapser  *m_optionsCollapser;

            QWidget            *m_previewFrame;
            KSaneViewer        *m_previewViewer;
            QWidget            *m_btnFrame;
            QToolButton        *m_zInBtn;
            QToolButton        *m_zOutBtn;
            QToolButton        *m_zSelBtn;
            QToolButton        *m_zFitBtn;
            KPushButton        *m_scanBtn;
            KPushButton        *m_prevBtn;

            QWidget            *m_activityFrame;
            QLabel             *m_warmingUp;
            QProgressBar       *m_progressBar;
            KPushButton        *m_cancelBtn;

            // device info
            SANE_Handle         m_saneHandle;
            QString             m_devName;
            QString             m_vendor;
            QString             m_model;

            // Option variables
            QList<KSaneOptInternal*> m_optList;
            QList<KSaneOptInternal*> m_pollList;
            KSaneOptInternal        *m_optSource;
            KSaneOptInternal        *m_optNegative;
            KSaneOptInternal        *m_optFilmType;
            KSaneOptInternal        *m_optMode;
            KSaneOptInternal        *m_optDepth;
            KSaneOptInternal        *m_optRes;
            KSaneOptInternal        *m_optResX;
            KSaneOptInternal        *m_optResY;
            KSaneOptInternal        *m_optTlX;
            KSaneOptInternal        *m_optTlY;
            KSaneOptInternal        *m_optBrX;
            KSaneOptInternal        *m_optBrY;
            KSaneOptInternal        *m_optPreview;
            KSaneOptInternal        *m_optGamR;
            KSaneOptInternal        *m_optGamG;
            KSaneOptInternal        *m_optGamB;
            KSaneCheckBox    *m_splitGamChB;
            KSaneGamma       *m_commonGamma;
            KSaneOptInternal        *m_optWaitForBtn;

            // preview variables
            qreal               m_previewWidth;
            qreal               m_previewHeight;
            qreal               m_previewDPI;
            QImage              m_previewImg;
            bool                m_isPreview;
            bool                m_autoSelect;

            int                 m_selIndex;

            bool                m_scanOngoing;
            bool                m_closeDevicePending;

            // final image data
            QByteArray          m_scanData;

            // option handling
            QTimer              m_readValsTmr;
            QTimer              m_updProgressTmr;
            QTimer              m_optionPollTmr;
            KSaneScanThread    *m_scanThread;
            KSanePreviewThread *m_previewThread;

            QString             m_saneUserName;
            QString             m_sanePassword;

            KSaneFindDevicesThread *m_findDevThread;
            KSaneAuth             *m_auth;
            KSaneWidget           *q;
    };

#endif
