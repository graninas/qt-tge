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


class GraphWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    struct GridSettings {
        double scale = 50.0; // Cell size in pixels
        QColor color = QColor(220, 220, 220); // Light gray
        QPointF center = QPointF(0, 0); // Center of the view in grid coordinates
    };

    QPointF dot1, dot2; // Dot positions
    QPointF viewDelta;  // Accumulated scene shift
    double viewScale = 1.0; // Accumulated zoom
    bool rightButtonPressed = false;
    QPoint lastMousePos;
    GridSettings gridSettings;

    int draggingDot = -1; // -1: none, 0: dot1, 1: dot2
    QPointF dragOffset;   // Offset from mouse to dot center
};

#endif // GRAPHWIDGET_H
