#include "graphwidget.h"
#include "graphwidget_helpers.h"
#include "tge/domain.h"
#include "gui_model.h"
#include "locationdialog.h"
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
    using graphwidget_helpers::isPointOnLocation;
    using graphwidget_helpers::locationTypeToString;

    if (event->button() == Qt::MiddleButton && model) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
        for (auto it = model->gameDef.locations.constBegin(); it != model->gameDef.locations.constEnd(); ++it) {
            int id = it.key();
            const auto& loc = it.value();
            if (graphwidget_helpers::isPointOnLocation(mouseScene, loc, step)) {
                // Show dialog
                QString typeStr = locationTypeToString(loc.type);
                LocationDialog dlg(loc, typeStr, this);
                if (dlg.exec() == QDialog::Accepted) {
                    // Save label
                    model->gameDef.locations[id].label = dlg.label();
                    // Save all descriptions
                    auto descs = dlg.descriptions();
                    auto& descPack = model->gameDef.locations[id].descriptionPack.descriptions;
                    descPack.clear();
                    for (const auto& d : descs) descPack.append(d);
                    viewport()->update();
                }
                event->accept();
                return;
            }
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
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
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

    // Edge hover detection (prototype: only for edge 2-3, straight)
    int newHoveredEdge = -1;
    if (model) {
        QTransform t;
        t.translate(viewDelta.x(), viewDelta.y());
        t.scale(viewScale, viewScale);
        QPointF mouseScene = t.inverted().map(event->pos());
        double step = gridSettings.scale;
        // Only check edge between 2 and 3
        auto eIt = model->gameDef.edges.constFind(0); // Find edge id 0 (or search for 2-3)
        for (auto eIt = model->gameDef.edges.constBegin(); eIt != model->gameDef.edges.constEnd(); ++eIt) {
            const auto& edge = eIt.value();
            if ((edge.fromLocation == 2 && edge.toLocation == 3) || (edge.fromLocation == 3 && edge.toLocation == 2)) {
                const auto& locs = model->gameDef.locations;
                auto from = locs.find(edge.fromLocation);
                auto to = locs.find(edge.toLocation);
                if (from != locs.end() && to != locs.end()) {
                    QPointF p1(from.value().coordX * step, from.value().coordY * step);
                    QPointF p2(to.value().coordX * step, to.value().coordY * step);
                    QLineF line(p1, p2);
                    // Manual segment distance (Qt < 5.14 compatible)
                    QPointF v = p2 - p1;
                    QPointF w = mouseScene - p1;
                    double c1 = QPointF::dotProduct(w, v);
                    double dist;
                    if (c1 <= 0) {
                        dist = QLineF(mouseScene, p1).length();
                    } else {
                        double c2 = QPointF::dotProduct(v, v);
                        if (c2 <= c1) {
                            dist = QLineF(mouseScene, p2).length();
                        } else {
                            double b = c1 / c2;
                            QPointF pb = p1 + b * v;
                            dist = QLineF(mouseScene, pb).length();
                        }
                    }
                    if (dist <= 8.0) {
                        newHoveredEdge = eIt.key();
                        break;
                    }
                }
            }
        }
    }
    if (newHoveredEdge != hoveredEdgeId) {
        hoveredEdgeId = newHoveredEdge;
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
        graphwidget_helpers::drawEdges(painter, model, gridSettings.scale, hoveredEdgeId);
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

