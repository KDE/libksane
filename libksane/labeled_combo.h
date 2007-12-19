/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007 by Kare Sars <kare dot sars at kolumbus dot fi>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef LABELED_COMBO_H
#define LABELED_COMBO_H

// Qt includes.

#include <QFrame>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>

namespace KSaneIface
{

/**
 * A label and a combobox.
 */
class LabeledCombo : public QFrame
{
    Q_OBJECT

public:
   /**
    * create a label and combobox combination.
    * \param parent parent widget
    * \param label is the lext for the label
    * \param list a stringlist with values the list should contain.
    */
    LabeledCombo(QWidget *parent, const QString& label, const QStringList& list);

   /**
    * \return the active text
    */
    QString currentText() const;

   /**
    * \return the text at position i
    */
    QString text(int i) const;

   /**
    * \return the number of list entries.
    */
    int count() const;
    void wigetSizeHints(int *lab_w, int *cmb_w);
    void setColumnWidths(int lab_w, int cmb_w);

public Q_SLOTS:

    /**
     * Add string entries to the combobox
     */
    void addItems(const QStringList &list) { combo->addItems(list); }
    
    /**
     * Remove all string entries
     */
    void clear() { combo->clear(); }

   /**
    * If the given string can be found in the comobox, activate that entry.
    * If not, the entry is not changed.
    */
    void setCurrentText(const QString &);

   /**
    * set the current item of the combobox.
    */
    void setCurrentIndex(int);

   /**
    * add an icon for a string in the combobox
    * @param icon the icon to set.
    * @param str the string for which the pixmap should be set.
    */
    bool setIcon(const QIcon& icon, const QString& str);

private Q_SLOTS:

    void prActivated(const QString &);
    void prActivated(int);

Q_SIGNALS:

    void activated(int);
    void activated(const QString&);

private:

    QLabel *label;
    QComboBox *combo;
    QGridLayout *layout;
};

}  // NameSpace KSaneIface

#endif // LABELED_COMBO_H
