// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/*
Gwenview: an image viewer
Copyright 2009 Aurélien Gâteau <agateau@kde.org>
Copyright 2009 Kåre Sårs <kare.sars@iki.fi>

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
// Self
#include "splittercollapser.moc"

// Qt
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QSplitter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTimeLine>

// KDE
#include <kdebug.h>

// Local

namespace KSane {

enum Direction {
	LTR      = 1 << 0,
	RTL      = 1 << 1,
	Vertical = 1 << 2,
	TTB      = Vertical + (1 << 0),
	BTT      = Vertical + (1 << 1)
};

const int TIMELINE_DURATION = 500;

const qreal MINIMUM_OPACITY = 0.3;

struct ArrowTypes {
	ArrowTypes() {}

	ArrowTypes(Qt::ArrowType t1, Qt::ArrowType t2)
	: visible(t1), notVisible(t2) {}

	Qt::ArrowType visible,
	notVisible;

	Qt::ArrowType get(bool isVisible) {
		return isVisible ? visible : notVisible;
	}
};


struct SplitterCollapserPrivate {
	SplitterCollapser* q;
	QSplitter* mSplitter;
	QWidget* mWidget;
	Direction mDirection;
	QTimeLine* mOpacityTimeLine;
	int mSizeAtCollaps;
	
	bool isVertical() const {
		return mDirection & Vertical;
	}
	
	bool isVisible() const {
		bool isVisible = mWidget->isVisible();
		QRect widgetRect = mWidget->geometry();
		if (isVisible) {
			QPoint br = widgetRect.bottomRight();
			if ((br.x() <= 0) || (br.y() <=0)) {
				isVisible = false;
			}
		}
		return isVisible;
	}
	
	void updatePosition() {
		int x, y;
		QRect widgetRect = mWidget->geometry();
		int splitterWidth = mSplitter->width();
		int handleWidth = mSplitter->handleWidth();
		int width = q->width();
		
		if (!isVertical()) {
			// FIXME: Make this configurable
			y = 30;
			if (mDirection == LTR) {
				if (isVisible()) {
					x = widgetRect.right() + handleWidth;
				} else {
					x = 0;
				}
			} else { // RTL
				if (isVisible()) {
					x = widgetRect.left() - handleWidth - width;
				} else {
					x = splitterWidth - handleWidth - width;
				}
			}
		} else {
			// FIXME
			x = 0;
			y = 0;
		}
		q->move(x, y);
	}


	void updateArrow() {
		static QMap<Direction, ArrowTypes> arrowForDirection;
		if (arrowForDirection.isEmpty()) {
			arrowForDirection[LTR] = ArrowTypes(Qt::LeftArrow,  Qt::RightArrow);
			arrowForDirection[RTL] = ArrowTypes(Qt::RightArrow, Qt::LeftArrow);
			arrowForDirection[TTB] = ArrowTypes(Qt::UpArrow,    Qt::DownArrow);
			arrowForDirection[BTT] = ArrowTypes(Qt::DownArrow,  Qt::UpArrow);
		}
		q->setArrowType(arrowForDirection[mDirection].get(isVisible()));
	}


	void widgetEventFilter(QEvent* event) {
		switch (event->type()) {
			case QEvent::Resize:
				updatePosition();
				updateOpacity();
				break;
				
			case QEvent::Move:
			case QEvent::Show:
			case QEvent::Hide:
				updatePosition();
				updateOpacity();
				updateArrow();
				break;
				
			default:
				break;
		}
	}


	void updateOpacity() {
		QPoint pos = q->parentWidget()->mapFromGlobal(QCursor::pos());
		QRect opaqueRect = q->geometry();
		bool opaqueCollapser = opaqueRect.contains(pos);
		int frame = mOpacityTimeLine->currentFrame();
		if (opaqueCollapser && frame == mOpacityTimeLine->startFrame()) {
			mOpacityTimeLine->setDirection(QTimeLine::Forward);
			startTimeLine();
		} else if (!opaqueCollapser && frame == mOpacityTimeLine->endFrame()) {
			mOpacityTimeLine->setDirection(QTimeLine::Backward);
			startTimeLine();
		}
	}


	void startTimeLine() {
		if (mOpacityTimeLine->state() != QTimeLine::Running) {
			mOpacityTimeLine->start();
		}
	}
};


SplitterCollapser::SplitterCollapser(QSplitter* splitter, QWidget* widget)
: QToolButton(),
d(new SplitterCollapserPrivate)
{
	d->q = this;

	// We do not want our collapser to be added as a regular widget in the
	// splitter!
	setAttribute(Qt::WA_NoChildEventsForParent);

	d->mOpacityTimeLine = new QTimeLine(TIMELINE_DURATION, this);
	d->mOpacityTimeLine->setFrameRange(int(MINIMUM_OPACITY * 1000), 1000);
	connect(d->mOpacityTimeLine, SIGNAL(valueChanged(qreal)), SLOT(update()));

	d->mWidget = widget;
	d->mWidget->installEventFilter(this);

	qApp->installEventFilter(this);

	d->mSplitter = splitter;
	setParent(d->mSplitter);

	if (splitter->indexOf(widget) < splitter->count() / 2) {
		d->mDirection = LTR;
	} else {
		d->mDirection = RTL;
	}
	if (splitter->orientation() == Qt::Vertical) {
		// FIXME: Ugly!
		d->mDirection = static_cast<Direction>(int(d->mDirection) + int(TTB));
	}

	connect(this, SIGNAL(clicked()), SLOT(slotClicked()));

	show();
}


SplitterCollapser::~SplitterCollapser() {
	delete d;
}


bool SplitterCollapser::eventFilter(QObject* object, QEvent* event) {
	if (object == d->mWidget) {
		d->widgetEventFilter(event);
	} else { /* app */
		if (event->type() == QEvent::MouseMove) {
			d->updateOpacity();
		}
	}
	return false;
}


QSize SplitterCollapser::sizeHint() const {
	int extent = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
	QSize sh(extent * 3 / 4, extent * 240 / 100);
	if (d->isVertical()) {
		sh.transpose();
	}
	return sh;
}


void SplitterCollapser::slotClicked() {
	QList<int> sizes = d->mSplitter->sizes();
	int index = d->mSplitter->indexOf(d->mWidget);
	if (d->isVisible()) {
		d->mSizeAtCollaps = sizes[index];
		sizes[index] = 0;
	}
	else {
		if (d->mSizeAtCollaps != 0) {
			sizes[index] = d->mSizeAtCollaps;
		}
		else {
			if (d->isVertical()) {
				sizes[index] = d->mWidget->sizeHint().height();
			}
			else {
				sizes[index] = d->mWidget->sizeHint().width();
			}
		}
	}
	d->mSplitter->setSizes(sizes);
}

void SplitterCollapser::slotCollapse() {
    if (d->isVisible()) slotClicked();
    // else do nothing
}

void SplitterCollapser::slotRestore() {
    if (!d->isVisible()) slotClicked();
    // else do nothing
}

void SplitterCollapser::slotSetCollapsed(bool collapse) {
    if (collapse == d->isVisible()) slotClicked();
    // else do nothing
}

void SplitterCollapser::paintEvent(QPaintEvent*) {
	QStylePainter painter(this);
	qreal opacity = d->mOpacityTimeLine->currentFrame() / 1000.;
	painter.setOpacity(opacity);

	QStyleOptionToolButton opt;
	initStyleOption(&opt);
	if (d->mDirection == LTR) {
		opt.rect.setLeft(-width());
	} else {
		opt.rect.setWidth(width() * 2);
	}
	painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

	QStyleOptionToolButton opt2;
	initStyleOption(&opt2);
	painter.drawControl(QStyle::CE_ToolButtonLabel, opt2);
}

}