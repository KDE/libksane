/*
 * SPDX-FileCopyrightText: 2002-2003 Stephan Stapel <stephan dot stapel at web dot de>
 * SPDX-FileCopyrightText: 2008-2009 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009, 2017 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef TWAINIFACE_H
#define TWAINIFACE_H

#include "twain.h"

// Windows includes
#include <windows.h>

#include <QWidget>

namespace KSaneIface
{
/**
 * Twain interface
 */
class KSaneWidgetPrivate : public QWidget
{
    Q_OBJECT

public:

    KSaneWidgetPrivate();
    ~KSaneWidgetPrivate();

    /**
     * Hook-in. See class documentation for details!
     * @result    One should return false to get the message being
     *            processed by the application (should return false by default!)
     */
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

    void CloseDSM();
    QString SelectSource();
    bool OpenSource(const QString &device);

public Q_SLOTS:
    bool OpenDialog();
    bool ReOpenDialog();

Q_SIGNALS:
    void ImageReady(QByteArray &data, int width, int height, int bytes_per_line, int format);
    void qImageReady(const QImage &image);

private:

    bool InitTwain();
    void ReleaseTwain();

    bool ProcessMessage(MSG msg);

    bool SetImageCount(TW_INT16 nCount = 1);
    bool DSIsOpen() const;

    bool CallTwainProc(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest,
                       TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG,
                       TW_MEMREF pData);

    bool EnableSource(bool showUI);
    void CloseDS();

    bool GetCapability(TW_CAPABILITY &twCap, TW_UINT16 cap, TW_UINT16 conType = TWON_DONTCARE16);
    bool GetCapability(TW_UINT16 cap, TW_UINT32 &value);
    bool SetCapability(TW_UINT16 cap, TW_UINT16 value, bool sign = false);
    bool SetCapability(TW_CAPABILITY &twCap);

    bool GetImageInfo(TW_IMAGEINFO &info);

    void TranslateMessage(TW_EVENT &twEvent);
    void TransferImage();
    bool EndTransfer();
    void CancelTransfer();
    bool GetImage(TW_IMAGEINFO &info);

    void ImageData(TW_MEMREF pdata, TW_IMAGEINFO &info);

protected:

    bool         m_bDSMOpen;
    bool         m_bDSOpen;
    bool         m_bSourceEnabled;
    bool         m_bModalUI;

    int          m_nImageCount;

    HINSTANCE    m_hTwainDLL;
    DSMENTRYPROC m_pDSMProc;

    TW_IDENTITY  m_AppId;
    TW_IDENTITY  m_Source;
    TW_STATUS    m_Status;
    TW_INT16     m_returnCode;
    HWND         m_hMessageWnd;
};

}

#endif /* TWAINIFACE_H */
