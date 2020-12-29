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

#include <QGroupBox>

#include <klocalizedstring.h>

#include <cmath>

namespace KSaneIface
{

LabeledGamma::LabeledGamma(QWidget *parent, const QString &text, int size, int max)
    : KSaneOptionWidget(parent, text)
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

    m_gammaTable.resize(size);
    for (int i = 0; i < m_gammaTable.size(); i++) {
        m_gammaTable[i] = i;
    }
    m_maxValue = max;

    m_gammaDisplay = new GammaDisp(this, &m_gammaTable, m_maxValue);

    QGroupBox *groupBox = new QGroupBox(text, this);
    QGridLayout *gr_lay = new QGridLayout(groupBox);

    gr_lay->addWidget(m_brightSlider, 0, 0);
    gr_lay->addWidget(m_contrastSlider, 1, 0);
    gr_lay->addWidget(m_gammaSlider, 2, 0);
    gr_lay->addWidget(m_gammaDisplay, 0, 1, 3, 1);

    m_label->hide();
    m_layout->addWidget(groupBox, 1, 0, 1, 3);

    connect(m_brightSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::calculateGT);
    connect(m_contrastSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::calculateGT);
    connect(m_gammaSlider, &LabeledSlider::valueChanged, this, &LabeledGamma::calculateGT);
}

LabeledGamma::~LabeledGamma()
{
}

void LabeledGamma::setColor(const QColor &color)
{
    if (m_gammaDisplay != nullptr) {
        m_gammaDisplay->setColor(color);
    }
}

void LabeledGamma::setValues(int bri, int con, int gam)
{
    m_brightSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_gammaSlider->blockSignals(true);

    m_brightSlider->setValue(bri);
    m_contrastSlider->setValue(con);
    m_gammaSlider->setValue(gam);

    calculateGT();

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
    int bri;
    int con;
    int gam;
    bool ok = true;

    gammaValues = values.split(QLatin1Char(':'));
    bri = gammaValues.at(0).toInt(&ok);
    if (ok) {
        con = gammaValues.at(1).toInt(&ok);
    }
    if (ok) {
        gam = gammaValues.at(2).toInt(&ok);
    }

    if (ok) {
        m_brightSlider->setValue(bri);
        m_contrastSlider->setValue(con);
        m_gammaSlider->setValue(gam);
        calculateGT();
    }

    m_brightSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_gammaSlider->blockSignals(false);
}

bool LabeledGamma::getValues(int &bri, int &con, int &gam)
{

    bri = m_brightSlider->value();
    con = m_contrastSlider->value();
    gam = m_gammaSlider->value();
    return true;
}

void LabeledGamma::setSize(int size)
{
    m_gammaTable.resize(size);
    for (int i = 0; i < m_gammaTable.size(); i++) {
        m_gammaTable[i] = i;
    }
    m_brightSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_gammaSlider->setValue(0);
}

const QVector<int> &LabeledGamma::gammaTablePtr()
{
    return m_gammaTable;
}

int LabeledGamma::size()
{
    return m_gammaTable.size();
}

int LabeledGamma::maxValue()
{
    return m_maxValue;
}


void LabeledGamma::calculateGT()
{
    double gam      = 100.0 / m_gammaSlider->value();
    double con      = (200.0 / (100.0 - m_contrastSlider->value())) - 1;
    double halfMax  = m_maxValue / 2.0;
    double bri      = (m_brightSlider->value() / halfMax) * m_maxValue;
    double x;

    for (int i = 0; i < m_gammaTable.size(); i++) {
        // apply gamma
        x = std::pow((double)i / m_gammaTable.size(), gam) * m_maxValue;

        // apply contrast
        x = (con * (x - halfMax)) + halfMax;

        // apply brightness + rounding
        x += bri + 0.5;

        // ensure correct value
        if (x > m_maxValue) {
            x = m_maxValue;
        }
        if (x < 0) {
            x = 0;
        }

        m_gammaTable[i] = (int)x;
    }

    m_gammaDisplay->update();
    Q_EMIT gammaChanged(m_brightSlider->value(), m_contrastSlider->value(), m_gammaSlider->value());
    Q_EMIT gammaTableChanged(m_gammaTable);
}

}  // NameSpace KSaneIface
