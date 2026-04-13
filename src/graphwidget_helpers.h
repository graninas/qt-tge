#ifndef GRAPHWIDGET_HELPERS_H
#define GRAPHWIDGET_HELPERS_H

#include <QPainter>
#include <QColor>
#include <QRectF>
#include "graphmodel.h"

namespace graphwidget_helpers {
constexpr int LOCATION_COLOR_COUNT = 15;
extern const QColor LOCATION_COLOR_PALETTE[LOCATION_COLOR_COUNT];

// Draw grid lines
void drawGrid(QPainter *painter, const QRectF &rect, double step, const QPointF &viewDelta, double viewScale);

// Draw all locations
void drawLocations(QPainter *painter, const GraphModel *model, double step, int idOffsetY, int labelOffsetY);

// Draw all edges
void drawEdges(QPainter *painter, const GraphModel *model, double step);
}

#endif // GRAPHWIDGET_HELPERS_H
