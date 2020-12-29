/* ============================================================
 *
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2008-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_BUTTON_H
#define KSANE_BUTTON_H

#include "ksaneoptionwidget.h"

// Qt includes
#include <QPushButton>
#include <QGridLayout>

namespace KSaneIface
{

/**
  *@author Kåre Särs
  */

/**
 * A wrapper for a checkbox
 */
class KSaneButton : public KSaneOptionWidget
{
    Q_OBJECT

public:
    /**
     * Create the checkbox.
     *
     * \param parent parent widget
     * \param text is the text describing the checkbox.
     */
    KSaneButton(QWidget *parent, const QString &text);
    ~KSaneButton();

Q_SIGNALS:
    void clicked();

private:
    QPushButton *m_button;
};

}  // NameSpace KSaneIface

#endif // KSANE_BUTTON_H
