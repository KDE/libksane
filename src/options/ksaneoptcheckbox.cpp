/* ============================================================
 *
 * This file is part of the KDE project
 *
 * Date        : 2009-01-21
 * Description : Sane interface for KDE
 *
 * SPDX-FileCopyrightText: 2009 Kare Sars <kare dot sars at iki dot fi>
 * SPDX-FileCopyrightText: 2014 Gregor Mitsch : port to KDE5 frameworks
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */
// Local includes
#include "ksaneoptcheckbox.h"

#include "labeledcheckbox.h"

#include <QtCore/QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOptCheckBox::KSaneOptCheckBox(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_checkbox(nullptr), m_checked(false)
{
}

void KSaneOptCheckBox::createWidget(QWidget *parent)
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

    m_widget = m_checkbox = new LabeledCheckbox(parent, sane_i18n(m_optDesc->title));
    m_widget->setToolTip(sane_i18n(m_optDesc->desc));

    connect(m_checkbox, &LabeledCheckbox::toggled, this, &KSaneOptCheckBox::checkboxChanged);

    updateVisibility();
    readValue();
}

void KSaneOptCheckBox::checkboxChanged(bool toggled)
{
    unsigned char data[4];

    m_checked = toggled;
    fromSANE_Word(data, (toggled) ? 1 : 0);
    writeData(data);
}

void KSaneOptCheckBox::readValue()
{
    if (state() == STATE_HIDDEN) {
        return;
    }

    // read the current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }
    bool old = m_checked;
    m_checked = (toSANE_Word(data.data()) != 0) ? true : false;
    if (m_checkbox) {
        m_checkbox->setChecked(m_checked);
    }
    if ((old != m_checked) && ((m_optDesc->cap & SANE_CAP_SOFT_SELECT) == 0)) {
        Q_EMIT buttonPressed(name(), sane_i18n(m_optDesc->title), m_checked);
    }
}

bool KSaneOptCheckBox::getValue(float &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    val = m_checked ? 1.0 : 0.0;
    return true;
}

bool KSaneOptCheckBox::setValue(float val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    checkboxChanged(val == 0);
    readValue();
    return true;
}

bool KSaneOptCheckBox::getValue(QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    val = m_checked ? QStringLiteral("true") : QStringLiteral("false");
    return true;
}

bool KSaneOptCheckBox::setValue(const QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    if ((val.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0) ||
            (val.compare(QStringLiteral("1")) == 0)) {
        checkboxChanged(true);
    } else {
        checkboxChanged(false);
    }
    readValue();
    return true;
}

bool KSaneOptCheckBox::hasGui()
{
    return true;
}

}  // NameSpace KSaneIface
