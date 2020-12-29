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
#include "ksaneoptentry.h"

#include "labeledentry.h"

#include <QtCore/QVarLengthArray>

#include <ksane_debug.h>

namespace KSaneIface
{

KSaneOptEntry::KSaneOptEntry(const SANE_Handle handle, const int index)
    : KSaneOption(handle, index), m_entry(nullptr)
{
}

void KSaneOptEntry::createWidget(QWidget *parent)
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

    m_widget = m_entry = new LabeledEntry(parent, sane_i18n(m_optDesc->title));
    m_widget->setToolTip(sane_i18n(m_optDesc->desc));
    connect(m_entry, &LabeledEntry::entryEdited, this, &KSaneOptEntry::entryChanged);
    updateVisibility();
    readValue();
}

void KSaneOptEntry::entryChanged(const QString &text)
{
    QString tmp;
    tmp += text.leftRef(m_optDesc->size);
    if (tmp != text) {
        m_entry->setText(tmp);
        writeData(tmp.toLatin1().data());
    }
}

void KSaneOptEntry::readValue()
{
    if (state() == STATE_HIDDEN) {
        return;
    }

    // read that current value
    QVarLengthArray<unsigned char> data(m_optDesc->size);
    SANE_Status status;
    SANE_Int res;
    status = sane_control_option(m_handle, m_index, SANE_ACTION_GET_VALUE, data.data(), &res);
    if (status != SANE_STATUS_GOOD) {
        return;
    }

    m_string = QString::fromUtf8(reinterpret_cast<char *>(data.data()));
    if (m_entry != nullptr) {
        m_entry->setText(m_string);
    }
}

bool KSaneOptEntry::getValue(float &)
{
    return false;
}
bool KSaneOptEntry::setValue(float)
{
    return false;
}

bool KSaneOptEntry::getValue(QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    val = m_string;
    return true;
}

bool KSaneOptEntry::setValue(const QString &val)
{
    if (state() == STATE_HIDDEN) {
        return false;
    }
    entryChanged(val);
    readValue();
    return true;
}

bool KSaneOptEntry::hasGui()
{
    return true;
}

}  // NameSpace KSaneIface
