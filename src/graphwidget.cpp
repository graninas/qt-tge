#include "graphwidget.h"
#include "graphwidget_helpers.h"
#include "tge/domain.h"
#include "gui_model.h"
#include "locationdialog.h"
#include "tge/editor/runtime/manager.h"
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

void GraphWidget::setModel(UiModel *m, const AppearanceSettings& appearance)
{
    model = m;
    appearanceSettings = appearance;
    centerOnObservedVirtualPoint();
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    double step = gridSettings.scale;
    if (event->button() == Qt::MiddleButton && model) {
        int id = graphwidget_helpers::findLocationAtMouse(model, event->pos(), viewDelta, viewScale, step);
        if (id != -1) {
            graphwidget_helpers::editLocationDialog(model, id, this, [this]() { viewport()->update(); });
            event->accept();
            return;
        }
    }
    if (event->button() == Qt::RightButton) {
        rightButtonPressed = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton && model) {
        QPointF mouseScene = graphwidget_helpers::mouseToScene(event->pos(), viewDelta, viewScale);
        for (auto it = model->gameDef.locations.constBegin(); it != model->gameDef.locations.constEnd(); ++it) {
            int id = it.key();
            const auto& loc = it.value();
            if (graphwidget_helpers::isPointOnLocation(mouseScene, loc, step)) {
                draggingDot = id;
                dragOffset = mouseScene - QPointF(loc.coordX * step, loc.coordY * step);
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
    bool hoverChanged = false;

    memoCursorPos = event->pos();

    if (rightButtonPressed)
    {
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
        model->gameDef.locations[draggingDot].coordX = newPos.x() / step;
        model->gameDef.locations[draggingDot].coordY = newPos.y() / step;
        viewport()->update();
        event->accept();
        return;
    }
    // Hover detection
    int newHovered = -1;
    if (model) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
        for (auto it = model->gameDef.locations.constBegin(); it != model->gameDef.locations.constEnd(); ++it) {
            int id = it.key();
            const auto& loc = it.value();
            if (graphwidget_helpers::isPointOnLocation(mouseScene, loc, step)) {
                newHovered = id;
                break;
            }
        }
    }
    if (newHovered != hoveredLocationId) {
        hoveredLocationId = newHovered;
        viewport()->update();
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
        model->gameDef.locations[draggingDot].coordX = std::round(model->gameDef.locations[draggingDot].coordX);
        model->gameDef.locations[draggingDot].coordY = std::round(model->gameDef.locations[draggingDot].coordY);
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
        graphwidget_helpers::drawLocations(painter, model, gridSettings.scale, appearanceSettings.idOffsetY, appearanceSettings.labelOffsetY, hoveredLocationId);
        // Draw memo on top if hovering
        if (hoveredLocationId != -1) {
            painter->save();
            painter->resetTransform(); // Draw memo in viewport coordinates
            const auto& loc = model->gameDef.locations[hoveredLocationId];
            QString typeStr = graphwidget_helpers::locationTypeToString(loc.type, true);
            QString desc = graphwidget_helpers::firstDescription(loc);
            graphwidget_helpers::drawLocationMemo(painter, loc, memoCursorPos, typeStr, desc);
            painter->restore();
        }
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

void GraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    double step = gridSettings.scale;
    if (event->button() == Qt::LeftButton && model) {
        int id = graphwidget_helpers::findLocationAtMouse(model, event->pos(), viewDelta, viewScale, step);
        if (id != -1) {
            graphwidget_helpers::editLocationDialog(model, id, this, [this]() { viewport()->update(); });
            event->accept();
            return;
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

