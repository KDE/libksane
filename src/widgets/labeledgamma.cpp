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

// Local includes
#include "labeledgamma.h"
#include "ksaneoption.h"

#include <QGroupBox>

#include <klocalizedstring.h>

namespace KSaneIface
{

LabeledGamma::LabeledGamma(QWidget *parent, const QString &text, int max)
    : KSaneOptionWidget(parent, text)
{
    initGamma(text, max);
}

LabeledGamma::LabeledGamma(QWidget *parent, KSaneOption *option)
    : KSaneOptionWidget(parent, option)
{

    int max = option->getMaxValue().toInt();
    initGamma(option->title(), max);
    connect(this, &LabeledGamma::valuesChanged, option, &KSaneOption::setValue);
    if (option->name() == QString::fromUtf8(SANE_NAME_GAMMA_VECTOR_R)) {
        setColor(Qt::red);
    }
    if (option->name() == QString::fromUtf8(SANE_NAME_GAMMA_VECTOR_G)) {
        setColor(Qt::green);
    }
    if (option->name() == QString::fromUtf8(SANE_NAME_GAMMA_VECTOR_B)) {
        setColor(Qt::blue);
    }
    setToolTip(option->description());
}

LabeledGamma::~LabeledGamma()
{
}

void LabeledGamma::initGamma(QString text, int max)
{
    m_brightSlider = new LabeledSlider(this, i18n("Brightness"), -50, 50, 1);
    m_brightSlider->setValue(0);

    m_contrastSlider = new LabeledSlider(this, i18n("Contrast"), -50, 50, 1);
    m_contrastSlider->setValue(0);

    m_gammaSlider = new LabeledSlider(this, i18n("Gamma"), 30, 300, 1);
    m_gammaSlider->setValue(100);

    // Calculate the size of the widgets in the sliders
    int labelMax = m_brightSlider->labelWidthHint();
    labelMax = qMax(labelMax, m_contrastSlider->labelWidthHint());
    labelMax = qMax(labelMax, m_gammaSlider->labelWidthHint());
    // set the calculated widths
    m_brightSlider->setLabelWidth(labelMax);
    m_contrastSlider->setLabelWidth(labelMax);
    m_gammaSlider->setLabelWidth(labelMax);
    
    m_maxValue = max;

    m_gammaDisplay = new GammaDisp(this, &m_brightness, &m_contrast, &m_gamma, max);

    QGroupBox *groupBox = new QGroupBox(text, this);
    QGridLayout *gr_lay = new QGridLayout(groupBox);

    gr_lay->addWidget(m_brightSlider, 0, 0);
    gr_lay->addWidget(m_contrastSlider, 1, 0);
    gr_lay->addWidget(m_gammaSlider, 2, 0);
    gr_lay->addWidget(m_gammaDisplay, 0, 1, 3, 1);

    m_label->hide();
    m_layout->addWidget(groupBox, 1, 0, 1, 3);

    connect(m_brightSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::emitNewValues);
    connect(m_contrastSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::emitNewValues);
    connect(m_gammaSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::emitNewValues);
    
    emitNewValues();
}

void LabeledGamma::setColor(const QColor &color)
{
    if (m_gammaDisplay != nullptr) {
        m_gammaDisplay->setColor(color);
    }
}

void LabeledGamma::setValues(int brightness, int contrast, int gamma)
{
    m_brightSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_gammaSlider->blockSignals(true);

    m_brightness = brightness;
    m_contrast = contrast;
    m_gamma = gamma;
    
    m_brightSlider->setValue(brightness);
    m_contrastSlider->setValue(contrast);
    m_gammaSlider->setValue(gamma);

    emitNewValues();
    
    m_brightSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_gammaSlider->blockSignals(false);
}

void LabeledGamma::setValues(const QString &values)
{
    m_brightSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_gammaSlider->blockSignals(true);

    QStringList gammaValues;
    int brightness;
    int contrast;
    int gamma;
    bool ok = true;

    gammaValues = values.split(QLatin1Char(':'));
    brightness = gammaValues.at(0).toInt(&ok);
    if (ok) {
        contrast = gammaValues.at(1).toInt(&ok);
    }
    if (ok) {
        gamma = gammaValues.at(2).toInt(&ok);
    }

    if (ok) {
        m_brightness = brightness;
        m_contrast = contrast;
        m_gamma = gamma;
        m_brightSlider->setValue(brightness);
        m_contrastSlider->setValue(contrast);
        m_gammaSlider->setValue(gamma);
        
        emitNewValues();
    }

    m_brightSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_gammaSlider->blockSignals(false);
}

void LabeledGamma::emitNewValues()
{
    m_brightness = m_brightSlider->value();
    m_contrast = m_contrastSlider->value();
    m_gamma = m_gammaSlider->value();
    QVector<int> values = { m_brightness, m_contrast, m_gamma };
    
    m_gammaDisplay->update();
    Q_EMIT gammaChanged(m_brightness, m_contrast, m_gamma);
    Q_EMIT valuesChanged(QVariant::fromValue(values));  
}

bool LabeledGamma::getValues(int &brightness, int &contrast, int &gamma)
{
    brightness = m_brightSlider->value();
    contrast = m_contrastSlider->value();
    gamma = m_gammaSlider->value();
    return true;
}

int LabeledGamma::maxValue()
{
    return m_maxValue;
}

}  // NameSpace KSaneIface
