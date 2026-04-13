#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QColor>
#include <QPointF>
#include <QPoint>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <cmath>
#include <QScrollBar>
#include "gui_model.h"


class UiModel;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);
    void setModel(UiModel *model, const AppearanceSettings& appearance = AppearanceSettings());

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void centerOnObservedVirtualPoint();

    struct GridSettings {
        double scale = 100.0; // Cell size in pixels
    };

    QPointF viewDelta;  // Accumulated scene shift
    double viewScale = 1.0; // Accumulated zoom
    bool rightButtonPressed = false;
    QPoint lastMousePos;
    GridSettings gridSettings;
    AppearanceSettings appearanceSettings;

    int draggingDot = -1; // -1: none, 0: dot1, 1: dot2
    QPointF dragOffset;   // Offset from mouse to dot center
    UiModel *model = nullptr;
};

#endif // GRAPHWIDGET_H
