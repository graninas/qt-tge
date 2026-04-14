#include <cmath>
#include "graphwidget_helpers.h"
#include <QFont>
#include <algorithm>
#include "tge/domain.h"
#include <QPainterPath>
#include <map>
#include <tuple>
#include <set>
#include "gui_model.h"

using namespace tge::domain;

namespace graphwidget_helpers {
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

void drawGrid(QPainter *painter, const QRectF &rect, double step, const QPointF &viewDelta, double viewScale) {
    painter->translate(viewDelta);
    painter->scale(viewScale, viewScale);
    QRectF r = painter->transform().inverted().mapRect(rect);
    int minX = static_cast<int>(std::floor(r.left() / step));
    int maxX = static_cast<int>(std::ceil(r.right() / step));
    int minY = static_cast<int>(std::floor(r.top() / step));
    int maxY = static_cast<int>(std::ceil(r.bottom() / step));
    for (int i = minX; i <= maxX; ++i) {
        double x = i * step;
        if (i == 0) {
            painter->setPen(QPen(Qt::black, 2));
        } else if (i % 10 == 0) {
            painter->setPen(QPen(Qt::gray, 2));
        } else if (i % 5 == 0) {
            painter->setPen(QPen(Qt::gray, 1));
        } else {
            painter->setPen(QPen(QColor(220, 220, 220), 1));
        }
        painter->drawLine(QLineF(x, r.top(), x, r.bottom()));
    }
    for (int j = minY; j <= maxY; ++j) {
        double y = j * step;
        if (j == 0) {
            painter->setPen(QPen(Qt::black, 2));
        } else if (j % 10 == 0) {
            painter->setPen(QPen(Qt::gray, 2));
        } else if (j % 5 == 0) {
            painter->setPen(QPen(Qt::gray, 1));
        } else {
            painter->setPen(QPen(QColor(220, 220, 220), 1));
        }
        painter->drawLine(QLineF(r.left(), y, r.right(), y));
    }
}

void drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to, double radius, double size) {
    // Compute direction
    QLineF line(from, to);
    if (line.length() == 0) return;
    // Move arrow tip to just outside the target node's circle
    line.setLength(line.length() - radius);
    QPointF tip = line.p2();
    double angle = std::atan2(-line.dy(), line.dx());
    QPointF left = tip + QPointF(-size * std::cos(angle - M_PI / 6), size * std::sin(angle - M_PI / 6));
    QPointF right = tip + QPointF(-size * std::cos(angle + M_PI / 6), size * std::sin(angle + M_PI / 6));
    QPolygonF arrowHead;
    arrowHead << tip << left << right;
    painter->setBrush(Qt::darkGreen);
    painter->drawPolygon(arrowHead);
}

void drawEdgeStraight(QPainter *painter, const UiModel *model, const tge::domain::EdgeDef &edge, double step) {
    const auto& locations = model->gameDef.locations;

    auto from = locations.find(edge.fromLocation);
    auto to = locations.find(edge.toLocation);

    if (from != locations.end() && to != locations.end()) {
        QPointF p1(from.value().coordX * step, from.value().coordY * step);
        QPointF p2(to.value().coordX * step, to.value().coordY * step);
        painter->drawLine(p1, p2);
        drawArrowHead(painter, p1, p2, 14.0, 14.0);
    }
}

// Draw a curved edge between two points, with a simple fixed offset
void drawEdgeCurvedSimple(QPainter *painter, const QPointF &p1, const QPointF &p2, double offset = 40.0) {
    QPointF mid = (p1 + p2) / 2.0;
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double length = std::sqrt(dx*dx + dy*dy);
    if (length == 0) return;
    double nx = -dy / length;
    double ny = dx / length;
    QPointF ctrl = mid + QPointF(nx * offset, ny * offset);
    painter->setPen(QPen(Qt::darkGreen, 2));
    painter->setBrush(Qt::NoBrush);
    QPainterPath path(p1);
    path.quadTo(ctrl, p2);
    painter->drawPath(path);
    drawArrowHead(painter, ctrl, p2, 14.0, 14.0);
}

// Helper: assign symmetric offsets to all edges between each unordered node pair
static void computeRepellingOffsets(const UiModel* model, std::map<int, double>& edgeOffset) {
    double baseOffset = 40.0;

    // Group all edges by unordered node pair
    std::map<std::pair<int, int>, std::vector<int>> pairToEdgeIds;
    for (auto it = model->gameDef.edges.constBegin(); it != model->gameDef.edges.constEnd(); ++it) {
        int i = it.key();
        const auto& edge = it.value();
        int a = std::min(edge.fromLocation, edge.toLocation);
        int b = std::max(edge.fromLocation, edge.toLocation);
        pairToEdgeIds[{a, b}].push_back(i);
    }

    // For each group, assign offsets regularly spaced in [-N/2, +N/2]
    for (const auto& group : pairToEdgeIds) {
        const auto& ids = group.second;
        int N = ids.size();
        std::vector<int> sortedIds = ids;
        std::sort(sortedIds.begin(), sortedIds.end());
        double start = -(N - 1) / 2.0;
        for (int idx = 0; idx < N; ++idx) {
            edgeOffset[sortedIds[idx]] = (start + idx) * baseOffset;
        }
    }
}

void drawEdges(QPainter *painter, const UiModel *model, double step) {
    std::map<int, double> edgeOffset;
    computeRepellingOffsets(model, edgeOffset);

    for (auto it = model->gameDef.edges.constBegin(); it != model->gameDef.edges.constEnd(); ++it) {
        int i = it.key();
        const auto& edge = it.value();
        int a = edge.fromLocation;
        int b = edge.toLocation;

        const auto& locations = model->gameDef.locations;

        auto from = locations.find(edge.fromLocation);
        auto to = locations.find(edge.toLocation);

        if (from != locations.end() && to != locations.end()) {
            QPointF p1(from.value().coordX * step, from.value().coordY * step);
            QPointF p2(to.value().coordX * step, to.value().coordY * step);

            double offset = edgeOffset[i];
            // Flip offset for reverse direction
            if (a > b) offset = -offset;

            if (offset == 0.0) {
                painter->setPen(QPen(Qt::darkGreen, 2));
                painter->drawLine(p1, p2);
                drawArrowHead(painter, p1, p2, 14.0, 14.0);
            } else {
                drawEdgeCurvedSimple(painter, p1, p2, offset);
            }
        }
    }
}

void drawLocations(QPainter *painter, const UiModel *model, double step, int idOffsetY, int labelOffsetY, int hoveredLocationId) {
    for (auto it = model->gameDef.locations.constBegin(); it != model->gameDef.locations.constEnd(); ++it) {
        const auto& loc = it.value();
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
            case tge::domain::LocationType::Start: fillColor = QColor(0, 200, 83); break;
            case tge::domain::LocationType::Finish: fillColor = QColor(255, 87, 34); break;
            case tge::domain::LocationType::Service: fillColor = QColor(54, 162, 235); break;
            default: fillColor = QColor(199, 199, 199); break;
        }
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(QBrush(fillColor));
        painter->drawEllipse(pos, 10, 10);
        // Draw hover effect
        if (hoveredLocationId == loc.id) {
            QPen hoverPen(Qt::darkGreen, 2, Qt::DashLine);
            painter->setPen(hoverPen);
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(pos, 18, 18);
        }
        // Draw id above
        painter->setPen(Qt::black);
        QFont font = painter->font();
        font.setBold(true);
        font.setPointSize(9);
        painter->setFont(font);
        QRectF idRect(pos.x() - 16, pos.y() + idOffsetY, 32, 14);
        painter->drawText(idRect, Qt::AlignCenter, QString::number(loc.id));
        // Draw label below (max 7 chars)
        QString label = loc.label;
        if (label.length() > 7) label = label.left(7) + "…";
        font.setBold(false);
        font.setPointSize(8);
        painter->setFont(font);
        QRectF labelRect(pos.x() - 16, pos.y() + labelOffsetY, 32, 14);
        painter->setPen(Qt::darkGray);
        painter->drawText(labelRect, Qt::AlignCenter, label);
    }
}
} // namespace graphwidget_helpers
