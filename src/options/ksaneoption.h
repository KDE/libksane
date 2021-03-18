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


class KSaneOption : public QObject
{
    Q_OBJECT

public:

    typedef enum {
        TypeDetectFail,
        TypeBool,
        TypeInteger,
        TypeFloat,
        TypeValueList,
        TypeString,
        TypeGamma,
        TypeAction
    } KSaneOptionType;

    Q_ENUM(KSaneOptionType);
    
    typedef enum {
        UnitNone,
        UnitPixel,
        UnitBit,
        UnitMilliMeter,
        UnitDPI,
        UnitPercent,
        UnitMicroSecond
    } KSaneOptionUnit;
    
    Q_ENUM(KSaneOptionUnit);
    
    typedef enum {
        StateHidden,
        StateDisabled,
        StateActive
    } KSaneOptionState;

    KSaneOption(const SANE_Handle handle, const int index);
    ~KSaneOption();
    static KSaneOptionType optionType(const SANE_Option_Descriptor *optDesc);

    bool needsPolling() const;
    KSaneOptionState state() const;
    QString name() const;
    QString title() const;
    QString description() const;
    KSaneOptionType type() const;

    virtual void readOption();
    virtual void readValue();

    virtual QVariant getMinValue() const;
    virtual QVariant getMaxValue() const;
    virtual QVariant getStepValue() const;
    virtual QVariant getValue() const;
    virtual QString getValueAsString() const;
    virtual QVariantList getEntryList() const;
    virtual KSaneOptionUnit getUnit() const;

    bool storeCurrentData();
    bool restoreSavedData();

Q_SIGNALS:
    void optionsNeedReload();
    void optionReloaded();
    void valuesNeedReload();
    void valueChanged(const QVariant &val);

public Q_SLOTS:
    
    virtual bool setValue(const QVariant &val); 
    
protected:

    static SANE_Word toSANE_Word(unsigned char *data);
    static void fromSANE_Word(unsigned char *data, SANE_Word from);
    bool writeData(void *data);

    SANE_Handle                   m_handle;
    int                           m_index;
    const SANE_Option_Descriptor *m_optDesc; ///< This pointer is provided by sane
    unsigned char                *m_data;
    KSaneOptionType               m_optionType = TypeDetectFail;
};



}  // NameSpace KSaneIface

#endif // KSANE_OPTION_H

