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

#ifndef KSaneOption_h
#define KSaneOption_h

//KDE includes
#include <KLocalizedString>

// Sane includes
extern "C"
{
    #include <sane/sane.h>
    #include <sane/saneopts.h>
}

class KSaneOption : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString strValue READ strValue WRITE setStrValue NOTIFY strValueChanged)
    Q_PROPERTY(qreal   value    READ value    WRITE setValue    NOTIFY valueChanged)

public:

    enum KSaneOptType {
        TYPE_DETECT_FAIL,
        TYPE_CHECKBOX,
        TYPE_SLIDER,
        TYPE_SLIDER_F,
        TYPE_COMBO,
        TYPE_ENTRY,
        TYPE_GAMMA,
        TYPE_BUTTON
    };

    enum Visibility {
        Hidden,
        Disabled,
        Shown
    };

    KSaneOption(const SANE_Handle handle, int index);
    ~KSaneOption();
    static KSaneOptType optionType(const SANE_Option_Descriptor *optDesc);

    bool needsPolling();
    Visibility visibility();

    /** Returns the technical name of the option */
    const QString saneName() const;

    /** Returns a translated title of the option for the user */
    const QString title() const;

    /** Returns a translated description of the option for the user */
    const QString description() const;

    virtual void readOption();
    virtual void readValue();

    virtual qreal minValue();
    virtual qreal maxValue();
    virtual qreal value();
    virtual const QString strValue();
    virtual bool setValue(qreal val);
    virtual bool setStrValue(const QString &val);
    virtual int  unit();
    virtual bool editable() {return false;}

    bool storeCurrentData();
    bool restoreSavedData();

Q_SIGNALS:
    void optsNeedReload();
    void valsNeedReload();

    void valueChanged();
    void strValueChanged();

protected:

    SANE_Word toSANE_Word(unsigned char *data);
    void fromSANE_Word(unsigned char *data, SANE_Word from);
    bool writeData(void *data);
    KLocalizedString unitString();
    QString unitDoubleString();
    void updateVisibility();

    SANE_Handle                   m_handle;
    int                           m_index;
    const SANE_Option_Descriptor *m_optDesc; ///< This pointer is provided by sane
    unsigned char                *m_data;
};

#endif // KSANE_OPTION_H
