/* ============================================================
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
    
    /** This enumeration describes the type of the option. */
    typedef enum {
        TypeDetectFail,
        TypeBool,
        TypeInteger,
        TypeDouble,
        TypeValueList,
        TypeString,
        TypeGamma,
        TypeAction
    } KSaneOptionType;

    Q_ENUM(KSaneOptionType);
    
    /** This enumeration describes the unit of the value of the option,
     * if any. */
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
    
    /** This enumeration describes the current statue of the value of 
     * the option, indicating if this option should be displayed or not. */
    typedef enum {
        StateHidden,
        StateDisabled,
        StateActive
    } KSaneOptionState;

    KSaneOption();
    KSaneOption(const SANE_Handle handle, const int index);
    ~KSaneOption();
    static KSaneOptionType optionType(const SANE_Option_Descriptor *optDesc);

    bool needsPolling() const;
    virtual void readOption();
    virtual void readValue();
    virtual QString valueAsString() const;
    
    /** This function returns the internal name of the option
     * @return the internal name */
    virtual QString name() const;
    
    /** This function returns the translated title of the option
     * @return the title */
    virtual QString title() const;

    /** This function returns a more verbose, translated description
     * of the option.
     * @return the description */
    virtual QString description() const;
    
    /** This function the type of the option as determined by libksane.
     * Each type provides a different implementation for different 
     * variable types, e.g. integer, float or string.
     * @return the type of option the of value KSaneOptionType */
    KSaneOptionType type() const;

    /** This function returns the state of the option indicating
     * if the function is disabled or should be hidden.
     * @return the state of option the of value KSaneOptionState */
    virtual KSaneOptionState state() const;
     
    /** This function returns the currently active value for the option.
     * @return the current value */  
    virtual QVariant value() const;
    
    /** This function returns the minimum value for the option.
     * Returns an empty QVariant if this value is not applicable
     * for the option type.
     * @return the minimum value */
    virtual QVariant minimumValue() const;
        
    /** This function returns the maximum value for the option.
     * Returns an empty QVariant if this value is not applicable
     * for the option type.
     * @return the maximum value */
    virtual QVariant maximumValue() const;
        
    /** This function returns the step value for the option.
     * Returns an empty QVariant if this value is not applicable
     * for the option type.
     * @return the step value */
    virtual QVariant stepValue() const;
    
    /** This function returns the list of possible values if the option
     * is of type KSaneOptionType::values. 
     * @return a list with all possible values */
    virtual QVariantList valueList() const;
    
    /** This function returns an enum specifying whether the values
     * of the option have a unit, e.g. mm, px, etc.
     * @return unit of value KSaneOptionUnit */
    virtual KSaneOptionUnit valueUnit() const;
    
    /** This function returns the size of the values of the option
     * of type KSaneOptionType::TypeValueList.
     * If the size is greater than one, value() and setValue() 
     * return and expect a QVariantList with an accordingly number
     * of elements. If the option is a KSaneOptionType::TypeString,
     * the size represents the number of characters in the string.
     * @return the number of elements */
    virtual int valueSize() const;

    /** This function temporarily stores the current value 
     * in a member variable. */
    bool storeCurrentData();
    
    /** This function restores the previously saved value 
     * and makes it the current value. */
    bool restoreSavedData();

Q_SIGNALS:
    /** This signal is emitted when the option is reloaded, which may
     * happen if the value of other options has changed. */ 
    void optionReloaded();
    
    /** This signal is emitted when the current value is updated, 
     * either when a user sets a new value or by a reload by the backend. */   
    void valueChanged(const QVariant &value);
    
    void optionsNeedReload();
    void valuesNeedReload();

public Q_SLOTS:
    
    virtual bool setValue(const QVariant &val); 
    
protected:

    static SANE_Word toSANE_Word(unsigned char *data);
    static void fromSANE_Word(unsigned char *data, SANE_Word from);
    bool writeData(void *data);

    SANE_Handle                   m_handle = nullptr;
    int                           m_index = -1;
    const SANE_Option_Descriptor *m_optDesc = nullptr; ///< This pointer is provided by sane
    unsigned char                *m_data= nullptr;
    KSaneOptionType               m_optionType = TypeDetectFail;
};



}  // NameSpace KSaneIface

#endif // KSANE_OPTION_H

