/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2011 by Kare Sars <kare.sars@iki .fi>
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

#ifndef KSaneEntry_h
#define KSaneEntry_h

#include "KSaneOptionWidget.h"

/**
 *@author Kåre Särs
 */

class QPushButton;
class KLineEdit;

/**
 * A text entry field with a set and reset button
 */
class KSaneEntry : public KSaneOptionWidget
{
    Q_OBJECT

public:

   /**
    * Create the entry.
    *
    * \param parent parent widget
    * \param text is the text describing the entry.
    */
    KSaneEntry(QWidget *parent, const QString& text);
    ~KSaneEntry();
    void setText(const QString& text);

private Q_SLOTS:

    void setClicked();
    void resetClicked();

Q_SIGNALS:

    void entryEdited(const QString& text);

private:

    KLineEdit *m_entry;
    QPushButton *m_set;
    QPushButton *m_reset;
    QString m_eText;
};

#endif
