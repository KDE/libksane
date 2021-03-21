/* ============================================================
 *
 * SPDX-FileCopyrightText: 2021 Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_PAGESIZE_OPTION_H
#define KSANE_PAGESIZE_OPTION_H

#include <QList>

#include "ksaneoption.h"

namespace KSaneIface
{

static const QString PageSizeOptionName = QStringLiteral("KSane::PageSize");    
    
class KSanePageSizeOption : public KSaneOption
{
    Q_OBJECT

public:
    KSanePageSizeOption(KSaneOption *m_optionTopLeftX, KSaneOption *m_optionTopLeftY,
                        KSaneOption *m_optionBottomRightX, KSaneOption *m_optionBottomRightY,
                        KSaneOption *m_optionResolution);

    QVariant value() const override;
    QString valueAsString() const override;
    
    KSaneOptionState state() const override;
    QString name() const override;
    QString title() const override;
    QString description() const override;
    QVariantList valueList() const override;

public Q_SLOTS:
    bool setValue(const QVariant &value) override;

private Q_SLOTS:
    void optionTopLeftXUpdated();
    void optionTopLeftYUpdated();
    void optionBottomRightXUpdated();
    void optionBottomRightYUpdated();
        
private:  
    double ensureMilliMeter(KSaneOption *option, double value);
    
    KSaneOption *m_optionTopLeftX;
    KSaneOption *m_optionTopLeftY;
    KSaneOption *m_optionBottomRightX;
    KSaneOption *m_optionBottomRightY;
    KSaneOption *m_optionResolution;
    int m_currentIndex = -1;
    KSaneOption::KSaneOptionState m_state = StateDisabled;
    QVariantList m_availableSizesListNames;
    QList<QSizeF> m_availableSizesList;
};  

}  // NameSpace KSaneIface

#endif // KSANE_PAGESIZE_OPTION_H
