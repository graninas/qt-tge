# Рефакторинг системы координат (src/ui/)

## Проблема
Работа с координатами в `graphwidget.cpp` и `graphwidget_helpers.cpp` была хаотичной:
- Смешивались три различные системы координат без явного разделения
- Преобразования координат разбросаны по коду
- Код было сложно понимать и поддерживать

## Решение: Промежуточный слой SceneModel

### Три системы координат

**1. Scene Coordinates (Координаты сцены) — Логические**
- Целые числа (ячейки сетки)
- Используются в `LocationDef.coordX` и `LocationDef.coordY`
- Представляют позицию в игровом мире
- Пример: Location находится в ячейке (2, 3)

**2. Canvas Coordinates (Координаты холста) — Графические**
- Пиксели, масштабированные согласно `gridStep`
- `canvas = scene * gridStep`
- Используются при отрисовке содержимого
- Применяются независимо от zoom/pan

**3. Widget Coordinates (Координаты виджета) — Экран**
- Пиксели на экране
- Применяется `viewDelta` (смещение) и `viewScale` (масштаб)
- Источник: координаты мыши в `QMouseEvent::pos()`

### Класс SceneModel (src/ui/gui_model.h)

Центральный контроллер преобразования координат:

```cpp
class SceneModel {
private:
  QPointF _viewDelta;           // Смещение камеры
  double _viewScale;            // Масштаб камеры (zoom)
  double _gridStep;             // Пиксели на ячейку сетки
  QPointF _sceneCenteredPoint;  // Центр в координатах сцены

public:
  // Основные преобразования
  QPointF sceneToCanvas(const QPointF& scenePoint) const;
  QPointF canvasToScene(const QPointF& canvasPoint) const;

  QPointF widgetToCanvas(const QPointF& widgetPoint) const;
  QPointF widgetToScene(const QPointF& widgetPoint) const;

  QPointF canvasToWidget(const QPointF& canvasPoint) const;
  QPointF sceneToWidget(const QPointF& scenePoint) const;

  QTransform canvasToWidgetTransform() const;

  // Accessors для параметров
  QPointF viewDelta() const;
  void setViewDelta(const QPointF& delta);

  double viewScale() const;
  void setViewScale(double scale);

  double gridStep() const;
  void setGridStep(double step);
};
```

### Обновленная архитектура

#### UiModel (gui_model.h)
```cpp
class UiModel {
public:
    SceneModel sceneModel;        // Центральный контроллер координат
    // ... остальные поля ...
};
```

#### GraphWidget (graphwidget.h/cpp)
**Удалено:**
- `QPointF viewDelta` → теперь в `model->sceneModel._viewDelta`
- `double viewScale` → теперь в `model->sceneModel._viewScale`
- `GridSettings gridSettings` → теперь в `model->sceneModel._gridStep`

**Сохранено:**
- Только UI-специфичные поля (cursor, hovered element, etc.)

#### Вспомогательные функции (graphwidget_helpers.h/cpp)

**До:**
```cpp
void drawGrid(QPainter *painter, const QRectF &rect,
              double step, const QPointF &viewDelta, double viewScale);

int findLocationAtMouse(const UiModel* model, const QPoint& mousePos,
                        const QPointF& viewDelta, double viewScale, double step);
```

**После:**
```cpp
void drawGrid(QPainter *painter, const QRectF &rect,
              const SceneModel* sceneModel);

int findLocationAtMouse(const UiModel* model, const QPoint& mousePos,
                        const SceneModel* sceneModel);
```

### Примеры использования

**Преобразование позиции мыши в координаты сцены:**
```cpp
// Старое
QTransform t;
t.translate(viewDelta.x(), viewDelta.y());
t.scale(viewScale, viewScale);
QPointF scenePoint = t.inverted().map(mousePos) / gridStep;

// Новое
QPointF scenePoint = model->sceneModel.widgetToScene(mousePos);
```

**Отрисовка элемента:**
```cpp
// Старое
QPointF pos(location.coordX * step, location.coordY * step);
painter.drawEllipse(pos, 10, 10);

// Новое
QPointF canvasPos = model->sceneModel.sceneToCanvas(
    QPointF(location.coordX, location.coordY));
painter.drawEllipse(canvasPos, 10, 10);
```

**Масштабирование (zoom):**
```cpp
// Старое
viewScale *= scaleFactor;

// Новое
model->sceneModel.setViewScale(model->sceneModel.viewScale() * scaleFactor);
```

## Преимущества

1. **Явное разделение** — Три системы координат четко разделены
2. **Централизованное управление** — Все преобразования в одном месте
3. **Переиспользуемость** — `SceneModel` можно использовать в других компонентах
4. **Тестируемость** — Преобразования координат легко протестировать отдельно
5. **Поддержка** — Код стал понятнее и проще для поддержки

## Файлы, затронутые рефакторингом

- [src/ui/gui_model.h](src/ui/gui_model.h) — Расширение SceneModel
- [src/ui/graphwidget.h](src/ui/graphwidget.h) — Удаление локальных полей
- [src/ui/graphwidget.cpp](src/ui/graphwidget.cpp) — Использование SceneModel
- [src/ui/graphwidget_helpers.h](src/ui/graphwidget_helpers.h) — Новые сигнатуры
- [src/ui/graphwidget_helpers.cpp](src/ui/graphwidget_helpers.cpp) — Использование SceneModel
- [src/ui/graphwidget_locations.cpp](src/ui/graphwidget_locations.cpp) — Использование SceneModel
- [src/ui/gui_model.cpp](src/ui/gui_model.cpp) — Обновление setSceneCenteredPoint

## Проверка

✅ Проект компилируется без ошибок
✅ Все целевые тесты компилируются
✅ Логика работы с координатами сохранена
