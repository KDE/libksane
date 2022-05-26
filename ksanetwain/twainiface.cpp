/*
 * SPDX-FileCopyrightText: 2002-2003 Stephan Stapel <stephan dot stapel at web dot de>
 * SPDX-FileCopyrightText: 2008-2009 Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009, 2017 Kare Sars <kare dot sars at iki dot fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

// I renamed the class to KSaneWidgetPrivate to remove
// the need for a wrapper class with this name. (Kare)

#include "twainiface.h"

#include "ksanewidget.h"

#define TWCPP_ANYCOUNT   (-1)
#define TWCPP_CANCELTHIS (1)
#define TWCPP_CANCELALL  (2)
#define TWCPP_DOTRANSFER (0)

#include <QString>
#include <QImage>
#include <QDebug>

#include <cstring>

#define saneDebug() if (0) qDebug()

namespace KSaneIface
{

KSaneWidgetPrivate::KSaneWidgetPrivate(): QWidget(0)
{
    // This is a dummy widget not visible. We use Qwidget to dispatch Windows event to
    // Twain interface. This is not possible to do it using QObject as well.

    m_hMessageWnd     = 0;
    m_hTwainDLL       = NULL;
    m_pDSMProc        = NULL;
    m_bDSOpen         = false;
    m_bDSMOpen        = false;
    m_bSourceEnabled  = false;
    m_bModalUI        = true;
    m_nImageCount     = TWCPP_ANYCOUNT;

    InitTwain();
}

KSaneWidgetPrivate::~KSaneWidgetPrivate()
{
    ReleaseTwain();
}

bool KSaneWidgetPrivate::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "windows_generic_MSG") {
        return ProcessMessage(*(MSG*)message);
    }
    return true;
}


/** Initializes TWAIN interface . Is already called from the constructor.
    It should be called again if ReleaseTwain is called.
    hWnd is the window which has to subclassed in order to receive
    Twain messaged. Normally - this would be your main application window.
 */
bool KSaneWidgetPrivate::InitTwain()
{
    char libName[512];

    if ((m_hTwainDLL && m_pDSMProc)) {
        return true;
    }

    memset(&m_AppId, 0, sizeof(m_AppId));

    if (!IsWindow((HWND)this->winId()))    {
        return false;
    }

    m_hMessageWnd = (HWND)this->winId();
    strcpy(libName, "TWAIN_32.DLL");

    m_hTwainDLL = LoadLibraryA(libName);
    if (m_hTwainDLL != NULL) {
        if (!(m_pDSMProc = (DSMENTRYPROC)GetProcAddress(m_hTwainDLL, (LPCSTR)MAKEINTRESOURCE(1)))) {
            FreeLibrary(m_hTwainDLL);
            m_hTwainDLL = NULL;
        }
    }

    if ((m_hTwainDLL && m_pDSMProc)) {
        // Expects all the fields in m_AppId to be set except for the id field.
        m_AppId.Id               = 0; // Initialize to 0 (Source Manager will assign real value)
        m_AppId.Version.MajorNum = 0; // Your app's version number
        m_AppId.Version.MinorNum = 2;
        m_AppId.Version.Language = TWLG_USA;
        m_AppId.Version.Country  = TWCY_USA;
        strcpy(m_AppId.Version.Info, "libksane");

        m_AppId.ProtocolMajor    = TWON_PROTOCOLMAJOR;
        m_AppId.ProtocolMinor    = TWON_PROTOCOLMINOR;
        m_AppId.SupportedGroups  = DG_IMAGE | DG_CONTROL;
        strcpy(m_AppId.Manufacturer,  "KDE");
        strcpy(m_AppId.ProductFamily, "Generic");
        strcpy(m_AppId.ProductName,   "libksane");

        m_bDSMOpen = CallTwainProc(&m_AppId, NULL, DG_CONTROL,
                                   DAT_PARENT, MSG_OPENDSM, (TW_MEMREF)&m_hMessageWnd);
        return true;
    } else {
        return false;
    }
}

/** Releases the twain interface . Need not be called unless you
    want to specifically shut it down.
 */
void KSaneWidgetPrivate::ReleaseTwain()
{
    if ((m_hTwainDLL && m_pDSMProc)) {
        CloseDSM();
        FreeLibrary(m_hTwainDLL);
        m_hTwainDLL = NULL;
        m_pDSMProc  = NULL;
    }
}

/** Entry point into Twain. For a complete description of this
    routine  please refer to the Twain specification 1.8
 */
bool KSaneWidgetPrivate::CallTwainProc(pTW_IDENTITY pOrigin, pTW_IDENTITY pDest,
                                       TW_UINT32 DG, TW_UINT16 DAT, TW_UINT16 MSG, TW_MEMREF pData)
{
    saneDebug() << "CallTwainProc" << pOrigin << pDest << DG << DAT << MSG << pData;
    if (!(m_hTwainDLL && m_pDSMProc)) {
        m_returnCode = TWRC_FAILURE;
        return false;
    }

    m_returnCode = (*m_pDSMProc)(pOrigin, pDest, DG, DAT, MSG, pData);

    if (m_returnCode == TWRC_FAILURE) {
        saneDebug() << "CallTwainProc m_returnCode == TWRC_FAILURE";
        (*m_pDSMProc)(pOrigin, pDest, DG_CONTROL, DAT_STATUS, MSG_GET, &m_Status);
    }
    return (m_returnCode == TWRC_SUCCESS);
}

/** Called to display a dialog box to select the Twain source to use.
    This can be overridden if a list of all sources is available
    to the application. These sources can be enumerated by Twain.
    it is not yet supported by KSaneWidgetPrivate.
 */
QString KSaneWidgetPrivate::SelectSource()
{
    TW_IDENTITY src;
    memset(&src, 0, sizeof(src));

    // debug printouts
    //bool ret_ok = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &src);
    //while (ret_ok) {
    //    saneDebug() << QLatin1String(src.ProductName);
    //    ret_ok = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &src);
    //}

    // set the default entry selected
    CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETDEFAULT, &src);

    // now open the selection dialog
    if (!CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_USERSELECT, &src)) {
        return QString();
    }

    QString source;
    source += QLatin1String(src.ProductName);
    source += QLatin1Char(';');
    source += QLatin1String(src.ProductFamily);
    source += QLatin1Char(';');
    source += QLatin1String(src.Manufacturer);
    saneDebug()<< source;
    return source;
}

/** Closes the Data Source
 */
void KSaneWidgetPrivate::CloseDS()
{
    saneDebug() << "CloseDS";
    if (DSIsOpen()) {
        if (m_bSourceEnabled) {
            TW_USERINTERFACE twUI;
            if (CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                              DAT_USERINTERFACE, MSG_DISABLEDS, &twUI)) {
                m_bSourceEnabled = false;
            }
        }
        CallTwainProc(&m_AppId, NULL, DG_CONTROL,
                      DAT_IDENTITY, MSG_CLOSEDS, (TW_MEMREF)&m_Source);
        m_bDSOpen = false;
    }
}

/** Closes the Data Source Manager */
void KSaneWidgetPrivate::CloseDSM()
{
    saneDebug() << "CloseDSM";
    if (m_hTwainDLL && m_pDSMProc && m_bDSMOpen) {
        CloseDS();
        CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_PARENT, MSG_CLOSEDSM, (TW_MEMREF)&m_hMessageWnd);
        m_bDSMOpen = false;
    }
}

/** Returns true if the Data Source is Open */
bool KSaneWidgetPrivate::DSIsOpen() const
{
    saneDebug() << "DSOpen:" << m_hTwainDLL << m_pDSMProc << m_bDSMOpen << m_bDSOpen;
    return (m_hTwainDLL && m_pDSMProc) && m_bDSMOpen && m_bDSOpen;
}

/** Opens a Data Source */
bool KSaneWidgetPrivate::OpenSource(const QString &source)
{
    saneDebug() << "OpenSource:" << source;
    if (source.isEmpty()) {
        return false;
    }

    QStringList splited = source.split(QLatin1Char(';'));
    if (splited.size() != 3) {
        return false;
    }

    // go thorough the list and check if the source is available
    bool ret_ok = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETFIRST, &m_Source);
    while (ret_ok) {
        saneDebug() << m_Source.Id << m_Source.Version.MajorNum << m_Source.Version.MinorNum;
        saneDebug() << m_Source.Manufacturer << m_Source.ProductFamily << m_Source.ProductName;
        if (QLatin1String(m_Source.ProductName) == splited[0]) {
            break;
        }
        ret_ok = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_GETNEXT, &m_Source);
    }

    if (!ret_ok) {
        saneDebug() << "CallTwainProc failed when reading beyond the last source";
        // CallTwainProc failed when reading beyond the last source
        return false;
    }

    // open the source
    if (m_hTwainDLL && m_pDSMProc && m_bDSMOpen) {
        m_bDSOpen = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, (TW_MEMREF)&m_Source);
        saneDebug() << "OpenSource(qst..) m_bDSOpen" <<  m_bDSOpen;
    }

    SetImageCount(TWCPP_ANYCOUNT);

    return DSIsOpen();
}

/** Re-Opens a Data Source */
bool KSaneWidgetPrivate::ReOpenDialog()
{
    saneDebug() << "ReOpenSource:" << m_hTwainDLL << m_pDSMProc << m_bDSMOpen << m_bDSOpen;

    if (DSIsOpen()) {
        // already open
        return true;
    }
    // open the source
    if (m_hTwainDLL && m_pDSMProc && m_bDSMOpen) {
        m_bDSOpen = CallTwainProc(&m_AppId, NULL, DG_CONTROL, DAT_IDENTITY, MSG_OPENDS, (TW_MEMREF)&m_Source);
        //saneDebug() << "ReOpenSource() m_bDSOpen" <<  m_bDSOpen;
    }

    SetImageCount(TWCPP_ANYCOUNT);

    if (DSIsOpen()) {
        return EnableSource(true);
    }
    //else
    return false;
}

/** Should be called from the main message loop of the application. Can always be called,
    it will not process the message unless a scan is in progress. */
bool KSaneWidgetPrivate::ProcessMessage(MSG msg)
{
    // TODO: don't really know why...
    if (msg.message == 528) {
        return false;
    }
    if (msg.message == 289) {
        return false;
    }

    if (m_hMessageWnd == 0) {
        return false;
    }

    saneDebug() << "ProcessMessage:" << msg.message << m_bSourceEnabled;

    if (m_bSourceEnabled)  {
        TW_UINT16  twRC = TWRC_NOTDSEVENT;

        TW_EVENT twEvent;
        twEvent.pEvent = (TW_MEMREF)&msg;
        //memset(&twEvent, 0, sizeof(TW_EVENT));
        twEvent.TWMessage = MSG_NULL;

        CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                      DAT_EVENT, MSG_PROCESSEVENT, (TW_MEMREF)&twEvent);

        saneDebug() << "ProcessMessage: retcode= " << m_returnCode;
        if (m_returnCode != TWRC_NOTDSEVENT) {
            TranslateMessage(twEvent);
        }
        return (twRC == TWRC_DSEVENT);
    }
    return false;
}

/** Queries the capability of the Twain Data Source */
bool KSaneWidgetPrivate::GetCapability(TW_CAPABILITY &twCap, TW_UINT16 cap, TW_UINT16 conType)
{
    saneDebug() << "GetCapability";
    if (DSIsOpen()) {
        twCap.Cap        = cap;
        twCap.ConType    = conType;
        twCap.hContainer = NULL;

        if (CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                          DAT_CAPABILITY, MSG_GET, (TW_MEMREF)&twCap)) {
            return true;
        }
    }
    return false;
}

/** Queries the capability of the Twain Data Source */
bool KSaneWidgetPrivate::GetCapability(TW_UINT16 cap, TW_UINT32 &value)
{
    saneDebug() << "GetCapability2";
    TW_CAPABILITY twCap;
    if (GetCapability(twCap, cap)) {
        pTW_ONEVALUE pVal;
        pVal = (pTW_ONEVALUE)GlobalLock(twCap.hContainer);

        if (pVal) {
            value = pVal->Item;
            GlobalUnlock(pVal);
            GlobalFree(twCap.hContainer);
            return true;
        }
    }
    return false;
}

/** Sets the capability of the Twain Data Source */
bool KSaneWidgetPrivate::SetCapability(TW_UINT16 cap, TW_UINT16 value, bool sign)
{
    saneDebug() << "SetCapability";
    if (DSIsOpen()) {
        TW_CAPABILITY twCap;
        pTW_ONEVALUE pVal;
        bool ret_value   = false;
        twCap.Cap        = cap;
        twCap.ConType    = TWON_ONEVALUE;
        twCap.hContainer = GlobalAlloc(GHND, sizeof(TW_ONEVALUE));

        if (twCap.hContainer) {
            pVal           = (pTW_ONEVALUE)GlobalLock(twCap.hContainer);
            pVal->ItemType = sign ? TWTY_INT16 : TWTY_UINT16;
            pVal->Item     = (TW_UINT32)value;
            GlobalUnlock(twCap.hContainer);
            ret_value      = SetCapability(twCap);
            GlobalFree(twCap.hContainer);
        }
        return ret_value;
    }
    return false;
}

/** Sets the capability of the Twain Data Source */
bool KSaneWidgetPrivate::SetCapability(TW_CAPABILITY &cap)
{
    saneDebug() << "SetCapability2";
    if (DSIsOpen()) {
        return CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                             DAT_CAPABILITY, MSG_SET, (TW_MEMREF)&cap);
    }
    return false;
}

/** Sets the number of images which can be accepted by the application at one time */
bool KSaneWidgetPrivate::SetImageCount(TW_INT16 nCount)
{
    saneDebug() << "SetImageCount" << nCount << (TW_UINT16)nCount;
    if (SetCapability(CAP_XFERCOUNT, (TW_UINT16)nCount, true)) {
        saneDebug() << "SetImageCount: nCount" << nCount;
        m_nImageCount = nCount;
        return true;
    } else {
        saneDebug() << "SetImageCount: failed:" << m_returnCode;
        if (m_returnCode == TWRC_CHECKSTATUS) {
            TW_UINT32 count;

            if (GetCapability(CAP_XFERCOUNT, count)) {
                nCount = (TW_INT16)count;

                if (SetCapability(CAP_XFERCOUNT, nCount)) {
                    m_nImageCount = nCount;
                    return true;
                }
            }
        }
    }
    return false;
}

/** Called to enable the Twain Acquire Dialog. This too can be
 * overridden but is a helluva job. */
bool KSaneWidgetPrivate::OpenDialog()
{
    saneDebug() << "OpenDialog";
    EnableSource(true);
    return true;
}

/** Called to enable the Twain Acquire Dialog. This too can be
 * overridden but is a helluva job. */
bool KSaneWidgetPrivate::EnableSource(bool showUI)
{
    saneDebug() << "EnableSource: DSIsOpen() =" << DSIsOpen();
    if (DSIsOpen() && !m_bSourceEnabled) {
        TW_USERINTERFACE twUI;
        twUI.ShowUI  = showUI;
        twUI.hParent = (TW_HANDLE)m_hMessageWnd;

        if (CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                          DAT_USERINTERFACE, MSG_ENABLEDS, (TW_MEMREF)&twUI)) {
            m_bSourceEnabled = true;
            m_bModalUI       = twUI.ModalUI;
        } else {
            m_bSourceEnabled = false;
            m_bModalUI       = true;
        }
        saneDebug() << "EnableSource: ModalUI=" << twUI.ModalUI << m_returnCode;
        return m_bSourceEnabled;
    }
    return false;
}

/** Called by ProcessMessage to Translate a TWAIN message */
void KSaneWidgetPrivate::TranslateMessage(TW_EVENT &twEvent)
{
    saneDebug() << "TranslateMessage: twEvent.TWMessage =" << twEvent.TWMessage;
    switch (twEvent.TWMessage) {
    case MSG_XFERREADY:
        saneDebug() << "MSG_XFERREADY";
        TransferImage();
        break;

    case MSG_CLOSEDSREQ:
        saneDebug() << "MSG_CLOSEDSREQ";
        CloseDS();
        break;
    }
}

/** Gets Imageinfo for an image which is about to be transferred. */
bool KSaneWidgetPrivate::GetImageInfo(TW_IMAGEINFO &info)
{
    saneDebug() << "GetImageInfo";
    if (m_bSourceEnabled) {
        return CallTwainProc(&m_AppId, &m_Source, DG_IMAGE,
                             DAT_IMAGEINFO, MSG_GET, (TW_MEMREF)&info);
    }
    return false;
}

/** Transfers the image or cancels the transfer depending on the state of the TWAIN system */
void KSaneWidgetPrivate::TransferImage()
{
    saneDebug() << "TransferImage()";
    TW_IMAGEINFO info;
    bool bContinue = true;

    while (bContinue) {
        if (GetImageInfo(info)) {
            int permission = TWCPP_DOTRANSFER;

            switch (permission) {
            case TWCPP_CANCELTHIS:
                bContinue = EndTransfer();
                break;

            case TWCPP_CANCELALL:
                CancelTransfer();
                bContinue = false;
                break;

            case TWCPP_DOTRANSFER:
                bContinue = GetImage(info);
                break;
            }
        }
    }
}

/** Ends the current transfer.
    Returns true if the more images are pending */
bool KSaneWidgetPrivate::EndTransfer()
{
    saneDebug() << "EndTransfer";
    TW_PENDINGXFERS twPend;
    if (CallTwainProc(&m_AppId, &m_Source, DG_CONTROL,
                      DAT_PENDINGXFERS, MSG_ENDXFER, (TW_MEMREF)&twPend)) {
        return twPend.Count != 0;
    }
    return false;
}

/** Aborts all transfers */
void KSaneWidgetPrivate::CancelTransfer()
{
    saneDebug() << "CancelTransfer";
    TW_PENDINGXFERS twPend;
    CallTwainProc(&m_AppId, &m_Source, DG_CONTROL, DAT_PENDINGXFERS, MSG_RESET, (TW_MEMREF)&twPend);
}

/** Calls TWAIN to actually get the image */
bool KSaneWidgetPrivate::GetImage(TW_IMAGEINFO &info)
{
    saneDebug() << "GetImage";
    TW_MEMREF pdata;
    CallTwainProc(&m_AppId, &m_Source, DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET, &pdata);

    switch (m_returnCode) {
    case TWRC_XFERDONE:
        saneDebug()<< "GetImage:TWRC_XFERDONE";
        ImageData(pdata, info);
        break;

    case TWRC_CANCEL:
        saneDebug()<< "GetImage:TWRC_CANCEL";
        break;

    case TWRC_FAILURE:
        saneDebug()<< "GetImage:TWRC_FAILURE";
        CancelTransfer();
        return false;
        break;

    default:
        saneDebug()<< "GetImage:" << m_returnCode;
    }

    GlobalFree(pdata);
    return EndTransfer();
}

void KSaneWidgetPrivate::ImageData(TW_MEMREF pdata, TW_IMAGEINFO &info)
{
    saneDebug() << "ImageData";
    if (pdata && (info.ImageWidth != -1) && (info.ImageLength != - 1)) {
        // Under Windows, Twain interface return a DIB data structure.
        // See http://en.wikipedia.org/wiki/Device-independent_bitmap#DIBs_in_memory for details.
        HGLOBAL hDIB     = (HGLOBAL)(qptrdiff)pdata;
        int size         = (int)GlobalSize(hDIB);
        const char *bits = (const char *)GlobalLock(hDIB);

        // DIB is BMP without header. we will add it to load data in QImage using std loader from Qt.
        QByteArray baBmp;
        QDataStream ds(&baBmp, QIODevice::WriteOnly);

        ds.writeRawData("BM", 2);

        qint32 filesize = size + 14;
        ds << filesize;

        qint16 reserved = 0;
        ds << reserved;
        ds << reserved;

        qint32 pixOffset = 14 + 40 + 0;
        ds << pixOffset;

        ds.writeRawData(bits, size);

        Q_EMIT ImageReady(baBmp, 0, 0, 0, (int)KSaneWidget::FormatBMP);
        Q_EMIT qImageReady(QImage::fromData(baBmp, "BMP"));

        GlobalUnlock(hDIB);

    }
}

}
