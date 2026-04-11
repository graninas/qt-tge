#ifndef TGE_PLAYER_TYPES_H
#define TGE_PLAYER_TYPES_H

#include <QVector>
#include <QString>
#include "../../tge/domain.h"

namespace tge {
namespace player {

// Forward declarations for static types
using tge::domain::VariableDef;
using tge::domain::LocationDef;
using tge::domain::EdgeDef;

// Dynamic variable state
struct VariableState {
    const VariableDef* def; // Reference to static definition
    QString value; // Current value (dummy for now)
};

// Dynamic edge state
struct EdgeState {
    const EdgeDef* def; // Reference to static definition
    const LocationState* toLocation = nullptr; // Pointer to dynamic destination location
};

// Dynamic location state
struct LocationState {
    const LocationDef* def; // Reference to static definition
    QVector<VariableState> localVariables;
    QVector<EdgeState> outgoingEdges;
};

// Dynamic game state (can be extended)
struct GameState {
    QVector<LocationState> locations;
    LocationState* startLocation = nullptr;
};

} // namespace player
} // namespace tge

#endif // TGE_PLAYER_TYPES_H
