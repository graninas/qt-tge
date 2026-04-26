#include "graphwidget_locations.h"
#include "graphwidget_helpers.h"
#include <QApplication>
#include <cmath>

namespace graphwidget_locations {

void handleNewLocationMode(GraphWidget* w, QMouseEvent* event) {
    if (!w->model) return;

    QPointF mouseScene = graphwidget_helpers::mouseToScene(event->pos(), &w->model->sceneModel);
    int gridX = std::round(mouseScene.x());
    int gridY = std::round(mouseScene.y());
    int newId = -1;
    if constexpr (std::is_member_object_pointer_v<decltype(&UiModel::manager)>) {
        auto& loc = w->model->manager.addLocation(QString(), 0, gridX, gridY);
        newId = loc.id;
    } else {
        newId = w->model->gameDef.locations.size() > 0 ? (w->model->gameDef.locations.lastKey() + 1) : 0;
        tge::domain::LocationDef loc;
        loc.coordX = gridX;
        loc.coordY = gridY;
        loc.type = tge::domain::LocationType::Regular;
        w->model->gameDef.locations[newId] = loc;
    }
    if (newId != -1) {
        const bool accepted = graphwidget_helpers::editLocationDialog(w->model, newId, w, [w]() { w->viewport()->update(); });
        if (!accepted) {
            w->model->manager.deleteLocation(newId);
            w->viewport()->update();
        }
        const bool ctrlPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
        w->setNewLocationMode(ctrlPressed);
    }
    event->accept();
}

void handleLocationDrag(GraphWidget* w, QMouseEvent* event) {
    if (!w->model) return;

    QPointF mouseScene = graphwidget_helpers::mouseToScene(event->pos(), &w->model->sceneModel);
    QPointF newPos = mouseScene - w->dragOffset;
    w->model->gameDef.locations[w->draggingDot].coordX = newPos.x();
    w->model->gameDef.locations[w->draggingDot].coordY = newPos.y();
    w->viewport()->update();
    event->accept();
}

void handleLocationHover(GraphWidget* w, QMouseEvent* event) {
    int newHovered = -1;
    if (w->model) {
        QPointF mouseCanvas = w->model->sceneModel.widgetToCanvas(event->pos());
        for (auto it = w->model->gameDef.locations.constBegin(); it != w->model->gameDef.locations.constEnd(); ++it) {
            int id = it.key();
            const auto& loc = it.value();
            if (graphwidget_helpers::isPointOnLocation(mouseCanvas, loc, &w->model->sceneModel)) {
                newHovered = id;
                break;
            }
        }
    }
    if (newHovered != w->hoveredLocationId) {
        w->hoveredLocationId = newHovered;
        w->viewport()->update();
    }
}

void handleLocationEdit(GraphWidget* w, int locationId) {
    graphwidget_helpers::editLocationDialog(w->model, locationId, w, [w]() { w->viewport()->update(); });
    const bool ctrlPressed = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    w->setNewLocationMode(ctrlPressed);
}

} // namespace graphwidget_locations
