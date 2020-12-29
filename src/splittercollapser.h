/*
Gwenview: an image viewer
SPDX-FileCopyrightText: 2009 Aurélien Gâteau <agateau@kde.org>

SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/
#ifndef SPLITTERCOLLAPSER_H
#define SPLITTERCOLLAPSER_H

// Qt
#include <QToolButton>

// KDE

class QSplitter;

namespace KSaneIface
{

struct SplitterCollapserPrivate;
/**
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 */
class SplitterCollapser : public QToolButton
{
    Q_OBJECT
public:
    SplitterCollapser(QSplitter *, QWidget *widget);
    ~SplitterCollapser();

    QSize sizeHint() const override;

public Q_SLOTS:
    void slotCollapse();
    void slotRestore();
    void slotSetCollapsed(bool collapsed);

protected:
    bool eventFilter(QObject *, QEvent *) override;

    void paintEvent(QPaintEvent *) override;

private:
    SplitterCollapserPrivate *const d;

private Q_SLOTS:
    void slotClicked();
};

} // namespace

#endif /* SPLITTERCOLLAPSER_H */
