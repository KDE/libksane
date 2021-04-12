/* ============================================================
 *
 * SPDX-FileCopyrightText: 2007-2011 Kare Sars <kare.sars@iki .fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */
#include "labeledcombo.h"

#include <QLabel>

#include <QComboBox>

namespace KSaneIface
{
    
static const char tmp_binary[] = "Binary";

LabeledCombo::LabeledCombo(QWidget *parent, const QString &ltext, const QStringList &list)
    : KSaneOptionWidget(parent, ltext)
{
    initCombo(list);
}

LabeledCombo::LabeledCombo(QWidget *parent, KSaneOption *option)
    : KSaneOptionWidget(parent, option)
{
    initCombo(QStringList());
    setLabelText(option->title());
    setToolTip(option->description());
    connect(this, &LabeledCombo::valueChanged, option, &KSaneOption::setValue);
    connect(option, &KSaneOption::valueChanged, this, &LabeledCombo::setValue);
    clear();
    
    const QVariantList list = option->getEntryList();
    for (const auto &value : list) {
        if (value.type() == static_cast<QVariant::Type>(QMetaType::Int)) {
            addItem(getStringWithUnitForInteger(value.toInt()), value);
        } else if (value.type() == static_cast<QVariant::Type>(QMetaType::Float)) {
            addItem(getStringWithUnitForFloat(value.toFloat()), value);
        } else {
            addItem(value.toString(), value);
        }
    }

    setIcon(QIcon::fromTheme(QStringLiteral("color")), QString::fromUtf8(SANE_VALUE_SCAN_MODE_COLOR));
    setIcon(QIcon::fromTheme(QStringLiteral("gray-scale")), QString::fromUtf8(SANE_VALUE_SCAN_MODE_GRAY));
    setIcon(QIcon::fromTheme(QStringLiteral("black-white")), QString::fromUtf8(SANE_VALUE_SCAN_MODE_LINEART));
    // The epkowa/epson backend uses "Binary" which is the same as "Lineart"
    setIcon(QIcon::fromTheme(QStringLiteral("black-white")), i18n(tmp_binary));
    QString currentText = option->getValue().toString();

    setCurrentText(currentText);
}

void LabeledCombo::initCombo(const QStringList &list)
{
    m_combo  = new QComboBox(this);
    m_combo->addItems(list);
    
    m_label->setBuddy(m_combo);

    connect(m_combo, QOverload<int>::of(&QComboBox::activated), this, &LabeledCombo::emitChangedValue);
    connect(m_combo, QOverload<int>::of(&QComboBox::activated), this, &LabeledCombo::activated);
    
    m_layout->addWidget(m_combo, 0, 1);
    m_layout->addWidget(new QWidget(this), 0, 2);
    m_layout->setColumnStretch(1, 0);
    m_layout->setColumnStretch(2, 50);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed); 
}

void LabeledCombo::addItems(const QStringList &list)
{
    m_combo->addItems(list);

    QString tmp;
    for (int i = 0; i < m_combo->count(); ++i) {
        tmp = m_combo->itemText(i);
        m_combo->setItemData(i, tmp, Qt::ToolTipRole);
    }
}

void LabeledCombo::setCurrentText(const QString &t)
{
    for (int i = 0; i < m_combo->count(); ++i) {
        if (m_combo->itemText(i) == t) {
            m_combo->setCurrentIndex(i);
        }
    }
}

QString LabeledCombo::currentText() const
{
    return m_combo->currentText();
}

bool LabeledCombo::setIcon(const QIcon &icon, const QString &str)
{
    for (int i = 0; i < m_combo->count(); ++i) {
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

void LabeledCombo::setValue(const QVariant &val)
{
    for (int i = 0; i < m_combo->count(); ++i) {
        if (m_combo->itemData(i) == val) {
            m_combo->setCurrentIndex(i);
            break;
        }
    }
}

void LabeledCombo::clear()
{
    m_combo->clear();
}

void LabeledCombo::emitChangedValue(int) 
{
    Q_EMIT valueChanged(m_combo->currentData());
}

QVariant LabeledCombo::currentData(int role) const
{
    return m_combo->currentData(role);
}

void LabeledCombo::addItem(const QString &text, const QVariant &userData)
{
    m_combo->addItem(text, userData);
}

int LabeledCombo::count() const
{
    return m_combo->count();
}

int LabeledCombo::currentIndex() const
{
    return m_combo->currentIndex();
}

QString LabeledCombo::getStringWithUnitForInteger(int iValue) const
{
    switch (m_option->getUnit()) {

    case KSaneOption::UnitPixel:
        return i18ncp("Parameter and Unit", "%1 Pixel", "%1 Pixels", iValue);
        break;
    case KSaneOption::UnitBit:
        return i18ncp("Parameter and Unit", "%1 Bit", "%1 Bits", iValue);
        break;
    case KSaneOption::UnitMilliMeter:
        return i18nc("Parameter and Unit (Millimeter)", "%1 mm", iValue);
        break;
    case KSaneOption::UnitDPI:
        return i18nc("Parameter and Unit (Dots Per Inch)", "%1 DPI", iValue);
        break;
    case KSaneOption::UnitPercent:
        return i18nc("Parameter and Unit (Percentage)", "%1 %", iValue);
        break;
    case KSaneOption::UnitMicroSecond:
        return i18nc("Parameter and Unit (Microseconds)", "%1 µs", iValue);
        break;
    default:
        return i18n("%1", iValue);
        break;
    }
}

QString LabeledCombo::getStringWithUnitForFloat(float fValue) const
{
    switch (m_option->getUnit()) {

    case KSaneOption::UnitPixel:
        return i18ncp("Parameter and Unit", "%1 Pixel", "%1 Pixels", static_cast<int>(fValue));
        break;
    case KSaneOption::UnitBit:
        return i18ncp("Parameter and Unit", "%1 Bit", "%1 Bits", static_cast<int>(fValue));
        break;
    case KSaneOption::UnitMilliMeter:
        return i18nc("Parameter and Unit (Millimeter)", "%1 mm", fValue);
        break;
    case KSaneOption::UnitDPI:
        return i18nc("Parameter and Unit (Dots Per Inch)", "%1 DPI", fValue);
        break;
    case KSaneOption::UnitPercent:
        return i18nc("Parameter and Unit (Percentage)", "%1 %", fValue);
        break;
    case KSaneOption::UnitMicroSecond:
        return i18nc("Parameter and Unit (Microseconds)", "%1 µs", fValue);
        break;
    default:
        return i18n("%1", fValue);
        break;
    }
}

}  // NameSpace KSaneIface
