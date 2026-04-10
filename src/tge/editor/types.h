#ifndef TGE_EDITOR_TYPES_H
#define TGE_EDITOR_TYPES_H

#include <QVector>
#include "../../tge/domain.h"

namespace tge {
namespace editor {

// Autoincrement ID generators for editor/construction
struct IdGenerator {
    int nextLocationId = 1;
    int nextEdgeId = 1;

    int generateLocationId() { return nextLocationId++; }
    int generateEdgeId() { return nextEdgeId++; }
};

// Editor/construction state (example, can be extended)
struct EditorState {
    // Optionally, can hold editor-specific state
};

} // namespace editor
} // namespace tge

#endif // TGE_EDITOR_TYPES_H
