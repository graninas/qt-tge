#include <cmath>
#include "graphwidget_helpers.h"
#include <QFont>
#include <algorithm>
#include "tge/domain.h"

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

void drawEdges(QPainter *painter, const GraphModel *model, double step) {
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
}

void drawLocations(QPainter *painter, const GraphModel *model, double step, int idOffsetY, int labelOffsetY) {
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
            case tge::domain::LocationType::Start: fillColor = QColor(0, 200, 83); break;
            case tge::domain::LocationType::Finish: fillColor = QColor(255, 87, 34); break;
            case tge::domain::LocationType::Service: fillColor = QColor(54, 162, 235); break;
            default: fillColor = QColor(199, 199, 199); break;
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
