#include "graphwidget.h"
#include "graphwidget_helpers.h"
#include "graphwidget_edges.h"
#include "graphwidget_locations.h"
#include "tge/domain.h"
#include "gui_model.h"
#include "locationdialog.h"
#include "tge/editor/runtime/manager.h"
#include "tge/domain.h"
#include "edgedialog.h"
#include <QToolButton> // Remove usage from code, keep for completeness>
#include <QWheelEvent>
#include <QPainter>
#include <cmath>
#include <QCursor>
#include <QToolTip>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include "graphwidget_errors.h"

GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent)
{
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setInteractive(false);
    setFocusPolicy(Qt::StrongFocus); // Accept key events
    viewDelta = QPointF(0, 0);
    viewScale = 1.0;
    rightButtonPressed = false;
    draggingDot = -1;
    model = nullptr;
    newLocationMode = false;
    edgeCreationState = EdgeCreationState::None;

    connect(&errorTimer, &QTimer::timeout, this, &GraphWidget::clearErrorMessage);
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

void GraphWidget::setNewLocationMode(bool enabled)
{
    newLocationMode = enabled;
    updateCursor();
}

void GraphWidget::updateCursor()
{
    if (newLocationMode) {
        QPixmap pix(32, 32);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::black, 2));
        p.drawEllipse(4, 4, 24, 24);
        p.end();
        setCursor(QCursor(pix));
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void GraphWidget::startEdgeCreation() { graphwidget_edges::startEdgeCreation(this); }
void GraphWidget::cancelEdgeCreation() { graphwidget_edges::cancelEdgeCreation(this); }
void GraphWidget::finishEdgeCreation(int destinationLocationId) { graphwidget_edges::finishEdgeCreation(this, destinationLocationId); }

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    if (edgeCreationState == EdgeCreationState::SelectSource && event->button() == Qt::LeftButton && model) {
        double step = gridSettings.scale;
        int id = graphwidget_helpers::findLocationAtMouse(model, event->pos(), viewDelta, viewScale, step);
        if (id != -1) {
            edgeSourceLocationId = id;
            edgeCreationState = EdgeCreationState::SelectDestination;
            edgeTempTarget = graphwidget_helpers::mouseToScene(event->pos(), viewDelta, viewScale);
            viewport()->update();
            event->accept();
            return;
        }
    } else if (edgeCreationState == EdgeCreationState::SelectDestination && event->button() == Qt::LeftButton && model) {
        double step = gridSettings.scale;
        int id = graphwidget_helpers::findLocationAtMouse(model, event->pos(), viewDelta, viewScale, step);
        if (id != -1 && id != edgeSourceLocationId) {
            finishEdgeCreation(id);
            event->accept();
            return;
        }
    }
    // Cancel edge creation on any other mouse button
    if (edgeCreationState != EdgeCreationState::None && event->button() != Qt::LeftButton) {
        cancelEdgeCreation();
        event->accept();
        return;
    }
    if (newLocationMode && event->button() == Qt::LeftButton && model) {
        graphwidget_locations::handleNewLocationMode(this, event);
        return;
    }
    double step = gridSettings.scale;
    if (event->button() == Qt::MiddleButton && model) {
        int id = graphwidget_helpers::findLocationAtMouse(model, event->pos(), viewDelta, viewScale, step);
        if (id != -1) {
            graphwidget_locations::handleLocationEdit(this, id);
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
        graphwidget_locations::handleLocationDrag(this, event);
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
    if (edgeCreationState == EdgeCreationState::SelectDestination) {
        edgeTempTarget = graphwidget_helpers::mouseToScene(event->pos(), viewDelta, viewScale);
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
    // Draw temporary edge if in SelectDestination
    if (edgeCreationState == EdgeCreationState::SelectDestination && model && edgeSourceLocationId != -1) {
        double step = gridSettings.scale;
        const auto& locations = model->gameDef.locations;
        auto it = locations.find(edgeSourceLocationId);
        if (it != locations.end()) {
            QPointF from(it.value().coordX * step, it.value().coordY * step);
            QPointF to = edgeTempTarget;
            painter.save();
            painter.translate(viewDelta);
            painter.scale(viewScale, viewScale);
            painter.setPen(QPen(Qt::darkGreen, 2, Qt::DashLine));
            painter.drawLine(from, to);
            graphwidget_helpers::drawArrowHead(&painter, from, to, 14.0, 14.0);
            painter.restore();
        }
    }
    // Draw error message if present
    drawErrorMessage(painter);
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
            graphwidget_locations::handleLocationEdit(this, id);
            event->accept();
            return;
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift && edgeCreationState == EdgeCreationState::None) {
        startEdgeCreation();
        event->accept();
        return;
    }
    // Cancel edge creation on any key except Shift during SelectDestination
    if (edgeCreationState == EdgeCreationState::SelectDestination && event->key() != Qt::Key_Shift) {
        cancelEdgeCreation();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Control) {
        setNewLocationMode(true);
    }
    QGraphicsView::keyPressEvent(event);
}

void GraphWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift && edgeCreationState != EdgeCreationState::None) {
        // If in SelectDestination, try to finish edge creation if cursor is over a location
        if (edgeCreationState == EdgeCreationState::SelectDestination && model) {
            double step = gridSettings.scale;
            int id = graphwidget_helpers::findLocationAtMouse(model, memoCursorPos, viewDelta, viewScale, step);
            if (id != -1 && id != edgeSourceLocationId) {
                finishEdgeCreation(id);
                event->accept();
                return;
            }
        }
        cancelEdgeCreation();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Control) {
        setNewLocationMode(false);
    }
    QGraphicsView::keyReleaseEvent(event);
}

void GraphWidget::showErrorMessage(const QString& msg, const QPoint& pos) {
    graphwidget_errors::showErrorMessage(this, msg, pos);
}

void GraphWidget::clearErrorMessage() {
    graphwidget_errors::clearErrorMessage(this);
}

QStringList GraphWidget::wrapErrorMessage(const QString& msg, int maxLineLen) const {
    return graphwidget_errors::wrapErrorMessage(msg, maxLineLen);
}

void GraphWidget::drawErrorMessage(QPainter& painter) const {
    graphwidget_errors::drawErrorMessage(this, painter);
}

