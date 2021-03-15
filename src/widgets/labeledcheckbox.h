/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef LABELED_CHECKBOX_H
#define LABELED_CHECKBOX_H

#include "ksaneoptionwidget.h"

// Qt includes
#include <QCheckBox>
#include <QGridLayout>

namespace KSaneIface
{

/**
  *@author Kåre Särs
  */

/**
 * A wrapper for a checkbox
 */
class LabeledCheckbox : public KSaneOptionWidget
{
    Q_OBJECT

public:

    /**
     * Create the checkbox.
     *
     * \param parent parent widget
     * \param text is the text describing the checkbox.
     */
    LabeledCheckbox(QWidget *parent, const QString &text);

    LabeledCheckbox(QWidget *parent, KSaneOption *option);
    ~LabeledCheckbox();
    void setChecked(bool);
    bool isChecked();

Q_SIGNALS:
    void toggled(bool);

public Q_SLOTS:
    void setValue(const QVariant &value);
    
protected:
    void initCheckBox(const QString &name);
    
private:

    QCheckBox *chbx;
};

}  // NameSpace KSaneIface

#endif // LABELED_CHECKBOX_H
