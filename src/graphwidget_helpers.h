#ifndef GRAPHWIDGET_HELPERS_H
#define GRAPHWIDGET_HELPERS_H

#include <QPainter>
#include <QColor>
#include <QRectF>

class UiModel;

namespace graphwidget_helpers {
constexpr int LOCATION_COLOR_COUNT = 15;
extern const QColor LOCATION_COLOR_PALETTE[LOCATION_COLOR_COUNT];

// Draw grid lines
void drawGrid(QPainter *painter, const QRectF &rect, double step, const QPointF &viewDelta, double viewScale);

// Draw all locations
void drawLocations(QPainter *painter, const UiModel *model, double step, int idOffsetY, int labelOffsetY);

// Draw all edges
void drawEdges(QPainter *painter, const UiModel *model, double step);
void drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to, double radius = 10.0, double size = 12.0);
}

#endif // GRAPHWIDGET_HELPERS_H
