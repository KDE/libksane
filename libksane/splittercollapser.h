// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef SplitterCollapser_h
#define SplitterCollapser_h

// Qt
#include <QToolButton>

// KDE

class QSplitter;

namespace KSane {


class SplitterCollapserPrivate;
/**
 * A button which appears on the side of a splitter handle and allows easy
 * collapsing of the widget on the opposite side
 */
class SplitterCollapser : public QToolButton {
	Q_OBJECT
public:
	SplitterCollapser(QSplitter*, QWidget* widget);
	~SplitterCollapser();

	virtual QSize sizeHint() const;

public Q_SLOTS:
    void slotCollapse();
    void slotRestore();
    void slotSetCollapsed(bool collapsed);
    
protected:
	virtual bool eventFilter(QObject*, QEvent*);

	virtual void paintEvent(QPaintEvent*);

private:
	SplitterCollapserPrivate* const d;

private Q_SLOTS:
	void slotClicked();
};


} // namespace

#endif
