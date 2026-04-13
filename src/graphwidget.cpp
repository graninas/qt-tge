#include "graphwidget.h"
#include "tge/domain.h"
#include <QWheelEvent>
#include <QPainter>
#include <cmath>

constexpr int LOCATION_COLOR_COUNT = 15;
const QColor LOCATION_COLOR_PALETTE[LOCATION_COLOR_COUNT] = {
    QColor(255, 99, 132),   // Red
    QColor(54, 162, 235),   // Blue
    QColor(255, 206, 86),   // Yellow
    QColor(75, 192, 192),   // Teal
    QColor(153, 102, 255),  // Purple
    QColor(255, 159, 64),   // Orange
    QColor(199, 199, 199),  // Grey
    QColor(255, 99, 255),   // Pink
    QColor(99, 255, 132),   // Light Green
    QColor(99, 132, 255),   // Light Blue
    QColor(255, 219, 88),   // Gold
    QColor(0, 200, 83),     // Green
    QColor(255, 87, 34),    // Deep Orange
    QColor(121, 85, 72),    // Brown
    QColor(233, 30, 99)     // Magenta
};

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

void GraphWidget::setModel(GraphModel *m)
{
    model = m;
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
    painter->translate(viewDelta);
    painter->scale(viewScale, viewScale);
    // Draw grid
    double step = gridSettings.scale;
    QRectF r = painter->transform().inverted().mapRect(rect);
    double x0 = std::floor(r.left() / step) * step;
    double y0 = std::floor(r.top() / step) * step;
    int minX = static_cast<int>(std::floor(r.left() / step));
    int maxX = static_cast<int>(std::ceil(r.right() / step));
    int minY = static_cast<int>(std::floor(r.top() / step));
    int maxY = static_cast<int>(std::ceil(r.bottom() / step));
    // Draw vertical grid lines
    for (int i = minX; i <= maxX; ++i) {
        double x = i * step;
        if (i == 0) {
            painter->setPen(QPen(Qt::black, 2)); // Y axis
        } else if (i % 10 == 0) {
            painter->setPen(QPen(Qt::gray, 2)); // Major
        } else if (i % 5 == 0) {
            painter->setPen(QPen(Qt::gray, 1)); // Medium
        } else {
            painter->setPen(QPen(gridSettings.color, 1)); // Minor
        }
        painter->drawLine(QLineF(x, r.top(), x, r.bottom()));
    }
    // Draw horizontal grid lines
    for (int j = minY; j <= maxY; ++j) {
        double y = j * step;
        if (j == 0) {
            painter->setPen(QPen(Qt::black, 2)); // X axis
        } else if (j % 10 == 0) {
            painter->setPen(QPen(Qt::gray, 2)); // Major
        } else if (j % 5 == 0) {
            painter->setPen(QPen(Qt::gray, 1)); // Medium
        } else {
            painter->setPen(QPen(gridSettings.color, 1)); // Minor
        }
        painter->drawLine(QLineF(r.left(), y, r.right(), y));
    }
    // Draw model if set
    if (model) {
        // Draw edges
        painter->setPen(QPen(Qt::darkGreen, 2));
        for (const auto &edge : model->edges) {
            const auto *from = std::find_if(model->locations.begin(), model->locations.end(), [&](const auto &l){return l.id == edge.fromLocation;});
            const auto *to = std::find_if(model->locations.begin(), model->locations.end(), [&](const auto &l){return l.id == edge.toLocation;});
            if (from != model->locations.end() && to != model->locations.end()) {
                QPointF p1(from->coordX * step, from->coordY * step);
                QPointF p2(to->coordX * step, to->coordY * step);
                painter->drawLine(p1, p2);
            }
        }
        // Draw locations
        for (const auto &loc : model->locations) {
            QPointF pos(loc.coordX * step, loc.coordY * step);
            // Draw color circle if color is set
            if (loc.color >= 0 && loc.color < LOCATION_COLOR_COUNT) {
                painter->setPen(QPen(LOCATION_COLOR_PALETTE[loc.color], 4));
            } else {
                painter->setPen(QPen(Qt::lightGray, 2));
            }
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(pos, 14, 14);
            // Fill color by type
            QColor fillColor;
            switch (loc.type) {
                case tge::domain::LocationType::Start: fillColor = QColor(0, 200, 83); break; // Green
                case tge::domain::LocationType::Finish: fillColor = QColor(255, 87, 34); break; // Orange
                case tge::domain::LocationType::Service: fillColor = QColor(54, 162, 235); break; // Blue
                default: fillColor = QColor(199, 199, 199); break; // Grey
            }
            painter->setPen(QPen(Qt::black, 2));
            painter->setBrush(QBrush(fillColor));
            painter->drawEllipse(pos, 10, 10);
            // Draw id above
            painter->setPen(Qt::black);
            QFont font = painter->font();
            font.setBold(true);
            font.setPointSize(9);
            painter->setFont(font);
            QRectF idRect(pos.x() - 16, pos.y() - 26, 32, 14);
            painter->drawText(idRect, Qt::AlignCenter, QString::number(loc.id));
            // Draw label below (max 7 chars)
            QString label = loc.label;
            if (label.length() > 7) label = label.left(7) + "…";
            font.setBold(false);
            font.setPointSize(8);
            painter->setFont(font);
            QRectF labelRect(pos.x() - 16, pos.y() + 12, 32, 14);
            painter->setPen(Qt::darkGray);
            painter->drawText(labelRect, Qt::AlignCenter, label);
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

