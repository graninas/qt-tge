#include "graphwidget.h"
#include "graphwidget_helpers.h"
#include "tge/domain.h"
#include <QWheelEvent>
#include <QPainter>
#include <cmath>

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setInteractive(false);
    viewDelta = QPointF(0, 0);
    viewScale = 1.0;
    rightButtonPressed = false;
    draggingDot = -1;
    model = nullptr;
}

void GraphWidget::centerOnObservedVirtualPoint()
{
    if (model) {
        double step = gridSettings.scale;
        QPointF center(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF virtualScene(model->observedVirtualPoint.x() * step, model->observedVirtualPoint.y() * step);
        viewDelta = center - virtualScene;
        viewport()->update();
    }
}

void GraphWidget::setModel(GraphModel *m, const AppearanceSettings& appearance)
{
    model = m;
    appearanceSettings = appearance;
    centerOnObservedVirtualPoint();
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
    if (event->button() == Qt::LeftButton && model) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
        for (int i = 0; i < model->locations.size(); ++i) {
            QPointF pos(model->locations[i].coordX * step, model->locations[i].coordY * step);
            if (QLineF(mouseScene, pos).length() <= 10) {
                draggingDot = i;
                dragOffset = mouseScene - pos;
                setCursor(Qt::OpenHandCursor);
                event->accept();
                return;
            }
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
    if (draggingDot != -1 && model) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
        QPointF newPos = mouseScene - dragOffset;
        model->locations[draggingDot].coordX = newPos.x() / step;
        model->locations[draggingDot].coordY = newPos.y() / step;
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
    if (event->button() == Qt::LeftButton && draggingDot != -1 && model) {
        double step = gridSettings.scale;
        // Snap to grid
        model->locations[draggingDot].coordX = std::round(model->locations[draggingDot].coordX);
        model->locations[draggingDot].coordY = std::round(model->locations[draggingDot].coordY);
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
    graphwidget_helpers::drawGrid(painter, rect, gridSettings.scale, viewDelta, viewScale);
    if (model) {
        graphwidget_helpers::drawEdges(painter, model, gridSettings.scale);
        graphwidget_helpers::drawLocations(painter, model, gridSettings.scale, appearanceSettings.idOffsetY, appearanceSettings.labelOffsetY);
    }
    painter->restore();
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    drawBackground(&painter, viewport()->rect());
}

void GraphWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    centerOnObservedVirtualPoint();
}

