#ifndef TGE_PLAYER_TYPES_H
#define TGE_PLAYER_TYPES_H

#include <vector>
#include <unordered_map>
#include <QString>
#include <memory>
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
    const struct LocationState* toLocation = nullptr; // Pointer to dynamic destination location
};

// Dynamic location state
struct LocationState {
    const LocationDef* def; // Reference to static definition
    std::vector<VariableState> localVariables;
    std::vector<EdgeState*> outgoingEdges;
    std::vector<EdgeState*> incomingEdges; // New: pointers to incoming edges
};

// Dynamic game state
struct GameState {
    std::unordered_map<int, std::unique_ptr<LocationState>> locations; // Map from id to unique_ptr<LocationState>
    std::unordered_map<int, std::unique_ptr<EdgeState>> edges; // Map from id to unique_ptr<EdgeState>
    LocationState* startLocation = nullptr;
    GameState() = default;
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;
    GameState(GameState&&) = default;
    GameState& operator=(GameState&&) = default;
};

} // namespace player
} // namespace tge

#endif // TGE_PLAYER_TYPES_H
