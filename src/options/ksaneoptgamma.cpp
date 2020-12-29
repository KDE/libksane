/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-31
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#include "ksaneoptgamma.h"

#include "labeledgamma.h"

#include <QtCore/QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOptGamma::KSaneOptGamma(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_gamma(nullptr)
{
}

void KSaneOptGamma::createWidget(QWidget *parent)
{
    if (m_widget) {
        return;
    }

    readOption();

    if (!m_optDesc) {
        qCDebug(KSANE_LOG) << "This is a bug";
        m_widget = new KSaneOptionWidget(parent, QString());
        return;
    }

    m_widget = m_gamma = new LabeledGamma(parent,
                                          sane_i18n(m_optDesc->title),
                                          m_optDesc->size / sizeof(SANE_Word),
                                          m_optDesc->constraint.range->max);
    connect(m_gamma, &LabeledGamma::gammaTableChanged, this, &KSaneOptGamma::gammaTableChanged);
    if (strcmp(m_optDesc->name, SANE_NAME_GAMMA_VECTOR_R) == 0) {
        m_gamma->setColor(Qt::red);
    }
    if (strcmp(m_optDesc->name, SANE_NAME_GAMMA_VECTOR_G) == 0) {
        m_gamma->setColor(Qt::green);
    }
    if (strcmp(m_optDesc->name, SANE_NAME_GAMMA_VECTOR_B) == 0) {
        m_gamma->setColor(Qt::blue);
    }

    m_widget->setToolTip(sane_i18n(m_optDesc->desc));
    updateVisibility();
    readValue();
}

void KSaneOptGamma::gammaTableChanged(const QVector<int> &gam_tbl)
{
    QVector<int> copy = gam_tbl;
    writeData(copy.data());
}

void KSaneOptGamma::readValue()
{
    // Unfortunately gamma table to brightness, contrast and gamma is
    // not easy nor fast.. ergo not done
}

bool KSaneOptGamma::getValue(float &)
{
    return false;
}
bool KSaneOptGamma::setValue(float)
{
    return false;
}

bool KSaneOptGamma::getValue(QString &val)
{
    if (!m_gamma) {
        return false;
    }
    if (state() == STATE_HIDDEN) {
        return false;
    }
    int bri;
    int con;
    int gam;
    m_gamma->getValues(bri, con, gam);
    val = QString::asprintf("%d:%d:%d", bri, con, gam);
    return true;
}

bool KSaneOptGamma::setValue(const QString &val)
{
    if (!m_gamma) {
        return false;
    }
    if (state() == STATE_HIDDEN) {
        return false;
    }

    m_gamma->setValues(val);
    return true;
}

bool KSaneOptGamma::hasGui()
{
    return true;
}

}  // NameSpace KSaneIface
