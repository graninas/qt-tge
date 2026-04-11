#include "graphwidget.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QWheelEvent>
#include <QPainter>
#include <cmath>

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent), gridSettings()
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    // Add two dots
    QGraphicsEllipseItem *dot1 = scene->addEllipse(-10, -10, 20, 20, QPen(Qt::black), QBrush(Qt::blue));
    QGraphicsEllipseItem *dot2 = scene->addEllipse(100-10, 100-10, 20, 20, QPen(Qt::black), QBrush(Qt::red));

    // Add a line connecting the dots
    scene->addLine(0, 0, 100, 100, QPen(Qt::darkGreen, 2));

    // Optional: center the view
    centerOn(50, 50);
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    // Zoom in/out
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Draw grid
    painter->save();
    painter->setPen(QPen(gridSettings.color, 1));
    double step = gridSettings.scale;
    QRectF r = rect;
    // Align grid to visible area
    double x0 = std::floor(r.left() / step) * step;
    double y0 = std::floor(r.top() / step) * step;
    for (double x = x0; x < r.right(); x += step) {
        painter->drawLine(QLineF(x, r.top(), x, r.bottom()));
    }
    for (double y = y0; y < r.bottom(); y += step) {
        painter->drawLine(QLineF(r.left(), y, r.right(), y));
    }
    painter->restore();
}
