/* ============================================================
*
* SPDX-FileCopyrightText: 2008 Kare Sars <kare dot sars at iki dot fi>
*
* SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*
* ============================================================ */

#ifndef KSANE_VIEWER_H
#define KSANE_VIEWER_H

#include <QGraphicsView>
#include <QWheelEvent>

namespace KSaneIface
{

/*
 * Preview image viewer that can handle a selection.
 */
class KSaneViewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit KSaneViewer(QImage *img, QWidget *parent = nullptr);
    ~KSaneViewer();

    void setQImage(QImage *img);
    void updateImage();
    /** Find selections in the picture
    * \param area this parameter determine the area of the reduced sized image. */
    void findSelections(float area = 10000.0);

    QSize sizeHint() const override;

public Q_SLOTS:

    void setTLX(float ratio);
    void setTLY(float ratio);
    void setBRX(float ratio);
    void setBRY(float ratio);

    /** This function is used to set a selection without the user setting it.
    * \note all parameters must be in the range 0.0 -> 1.0.
    * \param tl_x is the x coordinate of the top left corner 0=0 1=image with.
    * \param tl_y is the y coordinate of the top left corner 0=0 1=image height.
    * \param br_x is the x coordinate of the bottom right corner 0=0 1=image with.
    * \param br_y is the y coordinate of the bottom right corner 0=0 1=image height. */
    void setSelection(float tl_x, float tl_y, float br_x, float br_y);
    void clearActiveSelection();
    void clearSavedSelections();
    void clearSelections();

    /** This function is used to darken everything except what is inside the given area.
    * \note all parameters must be in the range 0.0 -> 1.0.
    * \param tl_x is the x coordinate of the top left corner 0=0 1=image with.
    * \param tl_y is the y coordinate of the top left corner 0=0 1=image height.
    * \param br_x is the x coordinate of the bottom right corner 0=0 1=image with.
    * \param br_y is the y coordinate of the bottom right corner 0=0 1=image height. */
    void setHighlightArea(float tl_x, float tl_y, float br_x, float br_y);

    /** This function sets the percentage of the highlighted area that is visible. The rest is hidden.
    * \param percentage is the percentage of the highlighted area that is shown.
    * \param hideColor is the color to use to hide the highlighted area of the image.*/
    void setHighlightShown(int percentage, QColor hideColor = Qt::white);

    /** This function removes the highlight area. */
    void clearHighlight();

    void zoomIn();
    void zoomOut();
    void zoomSel();
    void zoom2Fit();

    int selListSize();
    /* This function returns the active visible selection in index 0 and after that the "saved" ones */
    bool selectionAt(int index, float &tl_x, float &tl_y, float &br_x, float &br_y);

    void setMultiselectionEnabled(bool enabled);

Q_SIGNALS:
    void newSelection(float tl_x, float tl_y, float br_x, float br_y);

protected:
    void wheelEvent(QWheelEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void updateSelVisibility();
    void updateHighlight();
    bool activeSelection(float &tl_x, float &tl_y, float &br_x, float &br_y);
    void refineSelections(int pixelMargin);

    // fromRow is the row to start the iterations from. fromRow can be grater than toRow.
    // rowStart is the x1 coordinate of the row
    // all parameters are corrected to be valid pixel indexes,
    // but start must be < end
    int  refineRow(int fromRow, int toRow, int rowStart, int rowEnd);
    int  refineColumn(int fromCol, int toCol, int colStart, int colEnd);

    QPointF scenePos(QMouseEvent *e) const;

    struct Private;
    Private *const d;

};

}  // NameSpace KSaneIface

#endif

