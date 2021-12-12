/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef LABELED_COMBO_H
#define LABELED_COMBO_H

#include "ksaneoptionwidget.h"

#include <QVariant>

class QComboBox;

namespace KSaneIface
{

/**
 * A label and a combobox.
 */
class LabeledCombo : public KSaneOptionWidget
{
    Q_OBJECT

public:
    /**
     * create a label and combobox combination.
     * \param parent parent widget
     * \param label is the lext for the label
     * \param list a stringlist with values the list should contain.
     */
    LabeledCombo(QWidget *parent, const QString &label, const QStringList &list = QStringList());

    LabeledCombo(QWidget *parent, KSane::CoreOption *option);

    /** This function forwards the request to the QComboBox equivalent */
    QVariant currentData(int role = Qt::UserRole) const;

    /** This function forwards the request to the QComboBox equivalent */
    void addItem(const QString &text, const QVariant &userData = QVariant());

    /** This function forwards the request to the QComboBox equivalent */
    int count() const;

    /** This function forwards the request to the QComboBox equivalent */
    int currentIndex() const;

    /**
     * This function is used to read the current string of the combobox
     */
    QString currentText() const;

public Q_SLOTS:

    /**
     * Add string entries to the combobox
     */
    void addItems(const QStringList &list);

    /**
     * Remove all string entries
     */
    void clear();

    /**
     * If the given string can be found in the combobox, activate that entry.
     * If not, the entry is not changed.
     */
    void setCurrentText(const QString &);

    /**
     * set the current item of the combobox.
     */
    void setCurrentIndex(int);

    void setValue(const QVariant &val);

private Q_SLOTS:

      void emitChangedValue(int index);

Q_SIGNALS:

    void activated(int);

    void valueChanged(const QVariant &value);

private:
    void initCombo(const QStringList &list);

    QString getStringWithUnitForInteger(int iValue) const;

    QString getStringWithUnitForFloat(float iValue) const;

    QComboBox   *m_combo;
};

}  // NameSpace KSaneIface

#endif // LABELED_COMBO_H
