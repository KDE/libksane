/* ============================================================
 *
 * SPDX-FileCopyrightText: 2011 Kare Sars <kare.sars@iki.fi>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 * ============================================================ */

#ifndef KSANE_OPTION_WIDGET_H
#define KSANE_OPTION_WIDGET_H

// Qt includes
#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include <CoreOption>

namespace KSaneIface
{

/**
  *@author Kåre Särs
  */

/**
 * Base class for option widgets
 */
class KSaneOptionWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Create the Widget.
     * \param parent parent widget
     * \param labelText text.
     */
    KSaneOptionWidget(QWidget *parent, const QString &labelText);

    KSaneOptionWidget(QWidget *parent, KSane::CoreOption *option);
    ~KSaneOptionWidget() override;

    void setLabelText(const QString &text);

    int labelWidthHint();
    void setLabelWidth(int labelWidth);

protected Q_SLOTS:
    void updateVisibility();

protected:
    void initWidget();

    QLabel      *m_label;
    QGridLayout *m_layout;
    KSane::CoreOption *m_option = nullptr;
};

}  // NameSpace KSaneIface

#endif // KSANE_OPTION_WIDGET_H
