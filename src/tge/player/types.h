#ifndef TGE_PLAYER_TYPES_H
#define TGE_PLAYER_TYPES_H

#include <vector>
#include <unordered_map>
#include <QString>
#include <string>
#include <memory>
#include "../domain.h"

namespace tge {
namespace player {

// Forward declarations for static types
using tge::domain::VariableDef;
using tge::domain::LocationDef;
using tge::domain::EdgeDef;

// Dynamic variable state
struct VariableState {
    const VariableDef* def;
    QString value;
};

struct InfoDisplayItemState {
  const tge::domain::InfoDisplayItemDef *def;
  std::string value;
  bool visible;
  int priority;
};

// Dynamic edge state
struct EdgeState {
    const EdgeDef* def;
};

// Dynamic location state
struct LocationState {
    const LocationDef* def;
};

// Dynamic game state
struct GameState {
    std::unordered_map<int, std::unique_ptr<LocationState>> locations; // Map from id to unique_ptr<LocationState>
    std::unordered_map<int, std::unique_ptr<EdgeState>> edges; // Map from id to unique_ptr<EdgeState>
    std::vector<VariableState> variables; // Current variable values
    std::vector<InfoDisplayItemState> infoDisplayItems; // Current info display item states
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
