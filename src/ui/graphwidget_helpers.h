#ifndef GRAPHWIDGET_HELPERS_H
#define GRAPHWIDGET_HELPERS_H

#include <QPainter>
#include <QColor>
#include <QRectF>
#include "tge/domain.h"

class UiModel;
class SceneModel;

namespace graphwidget_helpers {
constexpr int LOCATION_COLOR_COUNT = 15;
extern const QColor LOCATION_COLOR_PALETTE[LOCATION_COLOR_COUNT];

// Draw grid lines using SceneModel for coordinate transformations
void drawGrid(QPainter *painter, const QRectF &rect, const SceneModel* sceneModel);

// Draw all locations using SceneModel for coordinate transformations
void drawLocations(QPainter *painter, const UiModel *model, const SceneModel* sceneModel, int idOffsetY, int labelOffsetY, int hoveredLocationId = -1);
void drawLocationMemo(QPainter* painter, const tge::domain::LocationDef& loc, const QPoint& pos, const QString& typeStr, const QString& desc);

// Draw all edges using SceneModel for coordinate transformations
void drawEdges(QPainter *painter, const UiModel *model, const SceneModel* sceneModel, int hoveredEdgeId = -1);
void drawArrowHead(QPainter *painter, const QPointF &from, const QPointF &to, double radius = 10.0, double size = 12.0);

// Check if canvas point is on location (canvas coordinates, not scene)
bool isPointOnLocation(const QPointF& canvasPoint, const tge::domain::LocationDef& loc, const SceneModel* sceneModel, double radius = 10.0);
QString firstDescription(const tge::domain::LocationDef& loc);
QString locationTypeToString(tge::domain::LocationType type, bool lower = false);

// Returns the id of the location under the given mouse position, or -1 if none
int findLocationAtMouse(const UiModel* model, const QPoint& mousePos, const SceneModel* sceneModel);

// Returns the id of the edge under the given mouse position, or -1 if none.
int findEdgeAtMouse(const UiModel* model, const QPoint& mousePos, const SceneModel* sceneModel, double hitWidth = 12.0);

// Transforms a mouse position to scene coordinates
QPointF mouseToScene(const QPoint& mousePos, const SceneModel* sceneModel);

// Opens the LocationDialog for the given location id, updates label and descriptions if accepted
void editLocationDialog(UiModel* model, int locationId, QWidget* parent, std::function<void()> onUpdate = nullptr);
}

#endif // GRAPHWIDGET_HELPERS_H
