#ifndef GRAPHWIDGET_HELPERS_H
#define GRAPHWIDGET_HELPERS_H

#include <QPainter>
#include <QColor>
#include <QRectF>
#include "tge/domain.h"

class UiModel;

namespace graphwidget_helpers {
constexpr int LOCATION_COLOR_COUNT = 15;
extern const QColor LOCATION_COLOR_PALETTE[LOCATION_COLOR_COUNT];

// Draw grid lines
void drawGrid(QPainter *painter, const QRectF &rect, double step, const QPointF &viewDelta, double viewScale);

// Draw all locations
void drawLocations(QPainter *painter, const UiModel *model, double step, int idOffsetY, int labelOffsetY, int hoveredLocationId = -1);
void drawLocationMemo(QPainter* painter, const tge::domain::LocationDef& loc, const QPoint& pos, const QString& typeStr, const QString& desc);

// Draw all edges
void drawEdges(QPainter *painter, const UiModel *model, double step);
void drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to, double radius = 10.0, double size = 12.0);

bool isPointOnLocation(const QPointF& scenePoint, const tge::domain::LocationDef& loc, double step, double radius = 10.0);
QString firstDescription(const tge::domain::LocationDef& loc);
QString locationTypeToString(tge::domain::LocationType type, bool lower = false);
}

#endif // GRAPHWIDGET_HELPERS_H
