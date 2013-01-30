/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Copyright (C) 2009-2013 by Kare Sars <kare.sars@iki.fi>
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

#ifndef KSaneOptInternal_h
#define KSaneOptInternal_h

#include <KLocalizedString>

// Sane includes
extern "C"
{
    #include <sane/sane.h>
    #include <sane/saneopts.h>
}

class KSaneOptInternal : public QObject
{
    Q_OBJECT

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

    KSaneOptInternal(const SANE_Handle handle, int index);
    ~KSaneOptInternal();

    static KSaneOptType optionType(const SANE_Option_Descriptor *optDesc);

    KSaneOptType optionType() const;

    bool needsPolling() const;
    Visibility visibility() const;

    /** Returns the technical name of the option */
    const QString saneName() const;

    /** Returns a translated title of the option for the user */
    const QString title() const;

    /** Returns a translated description of the option for the user */
    const QString description() const;

    const KLocalizedString unitString() const;
    const QString unitSpinBoxDoubleString() const;

    virtual void readOption();
    virtual void readValue();

    virtual qreal minValue() const;
    virtual qreal maxValue() const;
    virtual qreal value() const;
    virtual const QString strValue() const;
    virtual const QStringList comboStringList() const;

    virtual bool setValue(qreal val);
    virtual bool setStrValue(const QString &val);
    virtual int  unit() const;
    virtual bool editable() const {return false;}

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
    void updateVisibility();

    SANE_Handle                   m_handle;
    int                           m_index;
    const SANE_Option_Descriptor *m_optDesc; ///< This pointer is provided by sane
    unsigned char                *m_data;
};

#endif // KSANE_OPTION_H
