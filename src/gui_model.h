#ifndef GUI_MODEL_H
#define GUI_MODEL_H

#include "tge/domain.h"
#include <QPointF>
#include <QString>

struct AppearanceSettings {
    int idOffsetY;
    int labelOffsetY;
    AppearanceSettings(int idOffsetY_ = -46, int labelOffsetY_ = 32)
        : idOffsetY(idOffsetY_), labelOffsetY(labelOffsetY_) {}
};

// UI model containing observedVirtualPoint, appearance, and GameDef
class UiModel {
public:
    QPointF observedVirtualPoint = QPointF(0, 0);
    AppearanceSettings appearance;
    tge::domain::GameDef gameDef;

    static UiModel makeTestGraph();
};

#endif // GUI_MODEL_H
