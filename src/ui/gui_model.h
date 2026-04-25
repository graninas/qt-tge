#ifndef GUI_MODEL_H
#define GUI_MODEL_H

#include "tge/domain.h"
#include "tge/editor/runtime/manager.h"
#include "tge/editor/types.h"
#include <QPointF>
#include <QString>
#include <QTransform>

struct AppearanceSettings {
    int idOffsetY;
    int labelOffsetY;
    int customColorRingWidth;
    AppearanceSettings(int idOffsetY_ = -46, int labelOffsetY_ = 32, int customColorRingWidth_ = 7)
        : idOffsetY(idOffsetY_), labelOffsetY(labelOffsetY_), customColorRingWidth(customColorRingWidth_) {}
};



class SceneModel {
private:
  // Coordinate system parameters:
  // Scene coordinates: game world coordinates (integers in grid cells, e.g., location.coordX/Y)
  // Canvas coordinates: pixel coordinates for rendering (scene * gridStep)
  // Widget coordinates: screen pixel coordinates of mouse events

  QPointF _viewDelta;          // Canvas offset (translation)
  double _viewScale;           // Canvas zoom factor
  double _gridStep;            // Pixels per grid cell for canvas rendering
  QPointF _sceneCenteredPoint; // Center point in scene coordinates

public:
  SceneModel() :
    _viewDelta(0, 0),
    _viewScale(1.0),
    _gridStep(100.0),
    _sceneCenteredPoint(0, 0) {}

  // Accessors for view transformation
  QPointF viewDelta() const { return _viewDelta; }
  void setViewDelta(const QPointF& delta) { _viewDelta = delta; }

  double viewScale() const { return _viewScale; }
  void setViewScale(double scale) { _viewScale = scale; }

  double gridStep() const { return _gridStep; }
  void setGridStep(double step) { _gridStep = step; }

  // Scene centered point in scene coordinates
  QPointF sceneCenteredPoint() const { return _sceneCenteredPoint; }
  void setSceneCenteredPoint(const QPointF& point) { _sceneCenteredPoint = point; }

  // ============ Coordinate Transformations ============

  /// Convert scene coordinates (game world) to canvas coordinates (pixels for rendering)
  /// Scene coordinates are game cells (e.g., location.coordX/Y)
  /// Canvas coordinates are pixels where each cell = gridStep pixels
  QPointF sceneToCanvas(const QPointF& scenePoint) const {
      return scenePoint * _gridStep;
  }

  /// Convert canvas coordinates (pixels for rendering) to scene coordinates (game world)
  QPointF canvasToScene(const QPointF& canvasPoint) const {
      return canvasPoint / _gridStep;
  }

  /// Convert widget coordinates (mouse screen position) to canvas coordinates
  /// Widget coords are transformed by viewDelta and viewScale
  QPointF widgetToCanvas(const QPointF& widgetPoint) const {
      QTransform t;
      t.translate(_viewDelta.x(), _viewDelta.y());
      t.scale(_viewScale, _viewScale);
      return t.inverted().map(widgetPoint);
  }

  /// Convert widget coordinates to scene coordinates (game world)
  /// Direct transformation from mouse position to game cells
  QPointF widgetToScene(const QPointF& widgetPoint) const {
      return canvasToScene(widgetToCanvas(widgetPoint));
  }

  /// Convert canvas coordinates to widget coordinates
  QPointF canvasToWidget(const QPointF& canvasPoint) const {
      QTransform t;
      t.translate(_viewDelta.x(), _viewDelta.y());
      t.scale(_viewScale, _viewScale);
      return t.map(canvasPoint);
  }

  /// Convert scene coordinates to widget coordinates
  QPointF sceneToWidget(const QPointF& scenePoint) const {
      return canvasToWidget(sceneToCanvas(scenePoint));
  }

  /// Get the transformation matrix for rendering from canvas to widget
  QTransform canvasToWidgetTransform() const {
      QTransform t;
      t.translate(_viewDelta.x(), _viewDelta.y());
      t.scale(_viewScale, _viewScale);
      return t;
  }
};

// UI model
class UiModel {
public:
    SceneModel sceneModel;
    AppearanceSettings appearance;
    tge::domain::GameDef gameDef;
    tge::editor::EditorState editorState;
    tge::editor::IdGenerator idGen;
    tge::editor::runtime::Manager manager;

    UiModel() : manager(gameDef, editorState, idGen) {}

    static UiModel makeTestGraph();
};

#endif // GUI_MODEL_H
