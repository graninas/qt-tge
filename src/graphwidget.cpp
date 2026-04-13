#include "graphwidget.h"
#include <QWheelEvent>
#include <QPainter>
#include <cmath>

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setInteractive(false);
    // Initialize dot positions
    dot1 = QPointF(40, 40);
    dot2 = QPointF(90, 90);
    viewDelta = QPointF(0, 0);
    viewScale = 1.0;
    rightButtonPressed = false;
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        rightButtonPressed = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (rightButtonPressed) {
        QPoint delta = event->pos() - lastMousePos;
        viewDelta += QPointF(delta.x(), delta.y());
        lastMousePos = event->pos();
        viewport()->update();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        rightButtonPressed = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    // Custom zoom
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        viewScale *= scaleFactor;
    } else {
        viewScale /= scaleFactor;
    }
    viewport()->update();
}

void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    // Apply scale and translation
    painter->translate(viewDelta);
    painter->scale(viewScale, viewScale);
    // Draw grid
    painter->setPen(QPen(gridSettings.color, 1));
    double step = gridSettings.scale;
    QRectF r = painter->transform().inverted().mapRect(rect);
    double x0 = std::floor(r.left() / step) * step;
    double y0 = std::floor(r.top() / step) * step;
    for (double x = x0; x < r.right(); x += step) {
        painter->drawLine(QLineF(x, r.top(), x, r.bottom()));
    }
    for (double y = y0; y < r.bottom(); y += step) {
        painter->drawLine(QLineF(r.left(), y, r.right(), y));
    }
    // Draw dots and line
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::blue));
    painter->drawEllipse(dot1, 10, 10);
    painter->setBrush(QBrush(Qt::red));
    painter->drawEllipse(dot2, 10, 10);
    painter->setPen(QPen(Qt::darkGreen, 2));
    painter->drawLine(dot1, dot2);
    painter->restore();
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    drawBackground(&painter, viewport()->rect());
}

