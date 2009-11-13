/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2007-09-13
 * Description : Sane interface for KDE
 *
 * Copyright (C) 2007-2008 by Kare Sars <kare dot sars at iki dot fi>
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
#include "labeled_combo.h"
#include "labeled_combo.moc"

// Qt includes.
#include <QLabel>

// KDE includes
#include <KComboBox>
#include <KDebug>

namespace KSaneIface
{

LabeledCombo::LabeledCombo(QWidget *parent, const QString& ltext,
                           const QStringList& list)
            : QFrame(parent)
{
    m_layout = new QGridLayout(this);
    m_label  = new QLabel(ltext, this);
    m_combo  = new KComboBox(this);
    m_combo->addItems(list);

    m_label->setBuddy(m_combo);

    connect(m_combo, SIGNAL(activated(const QString &)),
            this, SLOT(prActivated(const QString &)));

    connect(m_combo, SIGNAL(activated(int)),
            this, SLOT(prActivated(int)));

    m_layout->setSpacing(3);
    m_layout->setMargin(0);
    m_layout->addWidget(m_label, 0, 0, Qt::AlignLeft);
    m_layout->addWidget(m_combo, 0, 2);
    m_layout->setColumnStretch(0, 0);
    m_layout->setColumnStretch(1, 50);
    m_layout->setColumnStretch(2, 0);
}

void LabeledCombo::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void LabeledCombo::setCurrentText(const QString &t)
{
    int i;
    for (i=0; i<m_combo->count(); i++) {
        if (m_combo->itemText(i) == t) {
            m_combo->setCurrentIndex(i);
        }
    }
}

QString LabeledCombo::currentText()
{
    return m_combo->currentText();
}


bool LabeledCombo::setIcon(const QIcon &icon, const QString& str)
{
    for (int i=0; i < m_combo->count(); i++) {
        if (m_combo->itemText(i) == str) {
            m_combo->setItemIcon(i, icon);
            return true;
        }
    }
    return false;
}

void LabeledCombo::setCurrentIndex(int i)
{
    m_combo->setCurrentIndex(i);
}


void LabeledCombo::widgetSizeHints(int *lab_w, int *cmb_w)
{
    if (lab_w != 0) *lab_w = m_label->sizeHint().width();
    if (cmb_w != 0) *cmb_w = m_combo->sizeHint().width();;
}

void LabeledCombo::setColumnWidths(int lab_w, int cmb_w)
{
    //kDebug() << "lab_w =" << lab_w << "cmb_w =" << cmb_w;
    m_layout->setColumnMinimumWidth(0, lab_w);
    m_layout->setColumnMinimumWidth(2, cmb_w);
}

void LabeledCombo::prActivated(const QString &t)
{
    emit activated(t);
}

void LabeledCombo::prActivated(int i)
{
    emit activated(i);
}

}  // NameSpace KSaneIface
