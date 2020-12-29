/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef LABELED_ENTRY_H
#define LABELED_ENTRY_H

#include "ksaneoptionwidget.h"

/**
 *@author Kåre Särs
 */

class QPushButton;
class QLineEdit;

namespace KSaneIface
{

/**
 * A text entry field with a set and reset button
 */
class LabeledEntry : public KSaneOptionWidget
{
    Q_OBJECT

public:

    /**
     * Create the entry.
     *
     * \param parent parent widget
     * \param text is the text describing the entry.
     */
    LabeledEntry(QWidget *parent, const QString &text);
    ~LabeledEntry();
    void setText(const QString &text);

private Q_SLOTS:

    void setClicked();
    void resetClicked();

Q_SIGNALS:

    void entryEdited(const QString &text);

private:

    QLineEdit *m_entry;
    QPushButton *m_set;
    QPushButton *m_reset;
    QString m_eText;
};

}  // NameSpace KSaneIface

#endif // LABELED_ENTRY_H
