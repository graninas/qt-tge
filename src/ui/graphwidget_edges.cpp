#include "graphwidget_edges.h"
#include "graphwidget_helpers.h"
#include "edgedialog.h"
#include <QDebug>

namespace graphwidget_edges {

void startEdgeCreation(GraphWidget* w) {
    w->edgeCreationState = GraphWidget::EdgeCreationState::SelectSource;
    w->edgeSourceLocationId = -1;
    w->edgeTempTarget = QPointF();
    w->viewport()->update();
}

void cancelEdgeCreation(GraphWidget* w) {
    w->edgeCreationState = GraphWidget::EdgeCreationState::None;
    w->edgeSourceLocationId = -1;
    w->edgeTempTarget = QPointF();
    w->viewport()->update();
}

void finishEdgeCreation(GraphWidget* w, int destinationLocationId) {
    if (!w->model || w->edgeSourceLocationId == -1 || destinationLocationId == -1)
        return;
    try {
        if (w->edgeSourceLocationId == destinationLocationId) {
            int serviceLocId = w->model->manager.addLoopEdge(w->edgeSourceLocationId, "", "");
            if (serviceLocId == -1) {
                qWarning() << "Loop edge creation error:" << w->model->manager.lastError();
                w->showErrorMessage(w->tr("Loop edge creation failed: ") + w->model->manager.lastError(), w->memoCursorPos);
            }
            w->viewport()->update();
        } else {
            auto* edge = w->model->manager.addEdge(w->edgeSourceLocationId, destinationLocationId, "", "");
            if (edge) {
                const int edgeId = edge->id;
                const bool accepted = graphwidget_helpers::editEdgeDialog(
                    w->model,
                    edgeId,
                    w,
                    [w]() { w->viewport()->update(); });
                if (!accepted) {
                    w->model->manager.deleteEdge(edgeId);
                }
            } else {
                qWarning() << "Edge creation error:" << w->model->manager.lastError();
                w->showErrorMessage(w->tr("Edge creation failed: ") + w->model->manager.lastError(), w->memoCursorPos);
            }
        }
    } catch (const std::exception& ex) {
        qWarning() << "Edge creation error:" << ex.what();
        w->showErrorMessage(w->tr("Edge creation failed: ") + ex.what(), w->memoCursorPos);
    }
    cancelEdgeCreation(w);
    w->viewport()->update();
}

} // namespace graphwidget_edges
