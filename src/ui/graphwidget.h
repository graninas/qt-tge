#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QColor>
#include <QPointF>
#include <QPoint>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <cmath>
#include <QScrollBar>
#include <QTimer>
#include "gui_model.h"
#include "graphwidget_errors.h"


class GraphWidget;
namespace graphwidget_edges {
    void startEdgeCreation(GraphWidget*);
    void cancelEdgeCreation(GraphWidget*);
    void finishEdgeCreation(GraphWidget*, int);
}
namespace graphwidget_locations {
    void handleNewLocationMode(GraphWidget*, QMouseEvent*);
    void handleLocationDrag(GraphWidget*, QMouseEvent*);
    void handleLocationHover(GraphWidget*, QMouseEvent*);
    void handleLocationEdit(GraphWidget*, int);
}

class UiModel;

class GraphWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);
    void setModel(UiModel *model, const AppearanceSettings& appearance = AppearanceSettings());
    void setNewLocationMode(bool enabled);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    static constexpr int clickDragThreshold = 4;

    void centerOnObservedVirtualPoint();
    void updateCursor();
    void clearSelection();
    bool toggleSelectionAtPosition(const QPoint& position);
    bool tryEditEdgeAtPosition(const QPoint& position);

    bool rightButtonPressed = false;
    QPoint lastMousePos;
    AppearanceSettings appearanceSettings;

    int draggingDot = -1; // -1: none, 0: dot1, 1: dot2
    QPointF dragOffset;   // Offset from mouse to dot center
    UiModel *model = nullptr;

    int hoveredLocationId = -1;
    int hoveredEdgeId = -1;
    QPoint leftPressPos;
    bool leftPressMoved = false;
    bool leftPressMayToggleSelection = false;
    QPoint memoCursorPos; // Screen position for memo
    bool newLocationMode = false;

    // --- New edge creation mode ---
    enum class EdgeCreationState { None, SelectSource, SelectDestination };
    EdgeCreationState edgeCreationState = EdgeCreationState::None;
    int edgeSourceLocationId = -1;
    QPointF edgeTempTarget; // Scene position of mouse during edge creation

    friend void graphwidget_edges::startEdgeCreation(GraphWidget*);
    friend void graphwidget_edges::cancelEdgeCreation(GraphWidget*);
    friend void graphwidget_edges::finishEdgeCreation(GraphWidget*, int);
    friend void graphwidget_locations::handleNewLocationMode(GraphWidget*, QMouseEvent*);
    friend void graphwidget_locations::handleLocationDrag(GraphWidget*, QMouseEvent*);
    friend void graphwidget_locations::handleLocationHover(GraphWidget*, QMouseEvent*);
    friend void graphwidget_locations::handleLocationEdit(GraphWidget*, int);

    void startEdgeCreation();
    void cancelEdgeCreation();
    void finishEdgeCreation(int destinationLocationId);

public:
    // Error message state
    void showErrorMessage(const QString& msg, const QPoint& pos);
    void clearErrorMessage();
    QStringList wrapErrorMessage(const QString& msg, int maxLineLen) const;
    void drawErrorMessage(QPainter& painter) const;

    // Make error handling members public for access by graphwidget_errors
    QString errorMessage;
    QPoint errorCursorPos;
    QTimer errorTimer;

signals:
    void newLocationCreated();
    void selectionChanged(int locationCount, int edgeCount);
};

#endif // GRAPHWIDGET_H
