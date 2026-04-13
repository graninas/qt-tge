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
    draggingDot = -1;
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
    if (event->button() == Qt::LeftButton) {
        // Transform mouse pos to logical (dot) coordinates
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        // Check if mouse is over a dot
        if (QLineF(mouseScene, dot1).length() <= 10) {
            draggingDot = 0;
            dragOffset = mouseScene - dot1;
            setCursor(Qt::OpenHandCursor);
            event->accept();
            return;
        } else if (QLineF(mouseScene, dot2).length() <= 10) {
            draggingDot = 1;
            dragOffset = mouseScene - dot2;
            setCursor(Qt::OpenHandCursor);
            event->accept();
            return;
        }
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
    if (draggingDot != -1) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        if (draggingDot == 0) {
            dot1 = mouseScene - dragOffset;
        } else if (draggingDot == 1) {
            dot2 = mouseScene - dragOffset;
        }
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
    if (event->button() == Qt::LeftButton && draggingDot != -1) {
        // Snap to grid
        double step = gridSettings.scale;
        if (draggingDot == 0) {
            dot1.setX(std::round(dot1.x() / step) * step);
            dot1.setY(std::round(dot1.y() / step) * step);
        } else if (draggingDot == 1) {
            dot2.setX(std::round(dot2.x() / step) * step);
            dot2.setY(std::round(dot2.y() / step) * step);
        }
        draggingDot = -1;
        setCursor(Qt::ArrowCursor);
        viewport()->update();
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
    // Draw labels
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(10);
    painter->setFont(font);
    QRectF labelRect1(dot1.x() - 8, dot1.y() - 8, 16, 16);
    QRectF labelRect2(dot2.x() - 8, dot2.y() - 8, 16, 16);
    painter->drawText(labelRect1, Qt::AlignCenter, "1");
    painter->drawText(labelRect2, Qt::AlignCenter, "2");
    painter->restore();
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    drawBackground(&painter, viewport()->rect());
}

