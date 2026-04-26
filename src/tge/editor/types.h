#ifndef TGE_EDITOR_TYPES_H
#define TGE_EDITOR_TYPES_H

#include <QVector>
#include "../../tge/domain.h"

namespace tge {
namespace editor {

enum class EditingMode {
    StaticModel,
    Player
};

// Centralized capability table used by UI forms/widgets to adapt editability.
struct CapabilityMatrix {
    bool allowLocationCreateDelete = true;
    bool allowLocationLabelEdit = true;
    bool allowLocationDescriptionEdit = true;
    bool allowLocationColorEdit = true;
    bool allowLocationMove = true;

    bool allowEdgeCreateDelete = true;
    bool allowEdgeOptionTextEdit = true;
    bool allowEdgeTransitionTextEdit = true;
    bool allowEdgeConditionEdit = true;
    bool allowEdgeVariableSettingsEdit = true;
    bool allowEdgeInfoDisplaySettingsEdit = true;
    bool allowEdgeColorEdit = true;
    bool allowEdgePriorityEdit = true;

    bool allowGlobalVariableCreateDelete = true;
    bool allowGlobalVariableIndexEdit = true;
    bool allowGlobalVariableNameEdit = true;
    bool allowGlobalVariableDescriptionEdit = true;
    bool allowGlobalVariableTypeEdit = true;
    bool allowGlobalVariableDefaultValueEdit = true;

    bool allowInfoDisplayItemCreateDelete = true;
    bool allowInfoDisplayItemLabelEdit = true;
    bool allowInfoDisplayItemFormulaEdit = true;
    bool allowInfoDisplayItemModeEdit = true;
    bool allowInfoDisplayItemPriorityEdit = true;
    bool allowInfoDisplayItemVisibilityEdit = true;
    bool allowInfoDisplayItemShowValueEdit = true;

    static CapabilityMatrix fullEditor() {
        return CapabilityMatrix();
    }

    static CapabilityMatrix playerMode() {
        CapabilityMatrix caps;

        caps.allowLocationCreateDelete = false;
        caps.allowLocationLabelEdit = true;
        caps.allowLocationDescriptionEdit = true;
        caps.allowLocationColorEdit = true;
        caps.allowLocationMove = true;

        caps.allowEdgeCreateDelete = false;
        caps.allowEdgeOptionTextEdit = true;
        caps.allowEdgeTransitionTextEdit = true;
        caps.allowEdgeConditionEdit = false;
        caps.allowEdgeVariableSettingsEdit = false;
        caps.allowEdgeInfoDisplaySettingsEdit = false;
        caps.allowEdgeColorEdit = true;
        caps.allowEdgePriorityEdit = false;

        caps.allowGlobalVariableCreateDelete = false;
        caps.allowGlobalVariableIndexEdit = false;
        caps.allowGlobalVariableNameEdit = true;
        caps.allowGlobalVariableDescriptionEdit = true;
        caps.allowGlobalVariableTypeEdit = false;
        caps.allowGlobalVariableDefaultValueEdit = false;

        caps.allowInfoDisplayItemCreateDelete = false;
        caps.allowInfoDisplayItemLabelEdit = false;
        caps.allowInfoDisplayItemFormulaEdit = false;
        caps.allowInfoDisplayItemModeEdit = false;
        caps.allowInfoDisplayItemPriorityEdit = false;
        caps.allowInfoDisplayItemVisibilityEdit = false;
        caps.allowInfoDisplayItemShowValueEdit = false;

        return caps;
    }

    bool canEditEdgeDialog() const {
        return allowEdgeOptionTextEdit
            || allowEdgeTransitionTextEdit
            || allowEdgeConditionEdit
            || allowEdgeVariableSettingsEdit
            || allowEdgeInfoDisplaySettingsEdit
            || allowEdgeColorEdit
            || allowEdgePriorityEdit;
    }

    bool canEditInfoDisplayItems() const {
        return allowInfoDisplayItemCreateDelete
            || allowInfoDisplayItemLabelEdit
            || allowInfoDisplayItemFormulaEdit
            || allowInfoDisplayItemModeEdit
            || allowInfoDisplayItemPriorityEdit
            || allowInfoDisplayItemVisibilityEdit
            || allowInfoDisplayItemShowValueEdit;
    }
};

// Autoincrement ID generators for editor/construction
struct IdGenerator {
    int nextLocationId = 1;
    int nextEdgeId = 1;

    int generateLocationId() { return nextLocationId++; }
    int generateEdgeId() { return nextEdgeId++; }
};

// Editor/construction state (example, can be extended)
struct EditorState {
    EditingMode mode = EditingMode::StaticModel;
    CapabilityMatrix capabilities = CapabilityMatrix::fullEditor();

    void setMode(EditingMode newMode) {
        mode = newMode;
        capabilities = (mode == EditingMode::Player)
            ? CapabilityMatrix::playerMode()
            : CapabilityMatrix::fullEditor();
    }
};

} // namespace editor
} // namespace tge

#endif // TGE_EDITOR_TYPES_H
