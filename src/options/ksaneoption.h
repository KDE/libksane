/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_OPTION_H
#define KSANE_OPTION_H

// Qt includes

#include <QFrame>

//KDE includes

#include <klocalizedstring.h>

// Sane includes
extern "C"
{
#include <sane/sane.h>
#include <sane/saneopts.h>
}

#define SANE_TRANSLATION_DOMAIN "sane-backends"

namespace KSaneIface
{

inline QString sane_i18n(const char *text) {
    return i18nd(SANE_TRANSLATION_DOMAIN, text);
}

class KSaneOptionWidget;

class KSaneOption : public QObject
{
    Q_OBJECT

public:

    typedef enum {
        TYPE_DETECT_FAIL,
        TYPE_CHECKBOX,
        TYPE_SLIDER,
        TYPE_F_SLIDER,
        TYPE_COMBO,
        TYPE_ENTRY,
        TYPE_GAMMA,
        TYPE_BUTTON
    } KSaneOptType;

    typedef enum {
        STATE_HIDDEN,
        STATE_DISABLED,
        STATE_SHOWN
    } KSaneOptWState;

    KSaneOption(const SANE_Handle handle, const int index);
    ~KSaneOption();
    static KSaneOptType optionType(const SANE_Option_Descriptor *optDesc);

    KSaneOptionWidget *widget();
    virtual bool hasGui();
    bool needsPolling() const;
    KSaneOptWState state() const;
    QString name() const;

    virtual void createWidget(QWidget *parent);

    virtual void readOption();
    virtual void readValue();

    virtual bool getMinValue(float &max);
    virtual bool getMaxValue(float &max);
    virtual bool getValue(float &val);
    virtual bool setValue(float val);
    virtual bool getValue(QString &val);
    virtual bool setValue(const QString &val);
    virtual int  getUnit();

    bool storeCurrentData();
    bool restoreSavedData();

Q_SIGNALS:
    void optsNeedReload();
    void valsNeedReload();
    void valueChanged();

protected:

    static SANE_Word toSANE_Word(unsigned char *data);
    static void fromSANE_Word(unsigned char *data, SANE_Word from);
    bool writeData(void *data);
    KLocalizedString unitString();
    QString unitDoubleString();
    void updateVisibility();

    SANE_Handle                   m_handle;
    int                           m_index;
    const SANE_Option_Descriptor *m_optDesc; ///< This pointer is provided by sane
    unsigned char                *m_data;
    KSaneOptionWidget            *m_widget;
};

}  // NameSpace KSaneIface

#endif // KSANE_OPTION_H

