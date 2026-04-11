#ifndef TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
#define TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H

#include <QString>
#include <optional>
#include "../../domain.h"
#include "../types.h"

namespace tge {
namespace player {
namespace runtime {

struct GameInitResult {
    std::optional<GameState> state;
    std::optional<QString> error;
};

class GameInitializer {
public:
    GameInitializer(const domain::GameDef& gameDef) : m_gameDef(gameDef) {}

    GameInitResult initialize() {
        GameInitResult result;
        // Example error check: must have at least one location
        if (m_gameDef.locations.isEmpty()) {
            result.error = QString("GameDef must have at least one location");
            return result;
        }
        GameState state;
        state.startLocation = nullptr;
        for (auto& locDef : m_gameDef.locations) {
            LocationState locState;
            locState.def = &locDef;
            for (const auto& varDef : locDef.localVariables) {
                VariableState varState;
                varState.def = &varDef;
                varState.value = "";
                locState.localVariables.append(varState);
            }
            // For each static outgoing edge, create an EdgeState (toLocation set later)
            for (const auto& edgeDef : locDef.outgoingEdges) {
                EdgeState edgeState;
                edgeState.def = &edgeDef;
                edgeState.toLocation = nullptr; // Will be set after all locations are created
                locState.outgoingEdges.append(edgeState);
            }
            // Set startLocation pointer while traversing
            if (!state.startLocation && locDef.type == domain::LocationType::Start) {
                state.locations.append(locState);
                state.startLocation = &state.locations.last();
            } else {
                state.locations.append(locState);
            }
        }
        // Set toLocation pointers for all edges
        // FIXME: This is O(N^2) and inefficient for large graphs. Consider optimizing with a map from id to LocationState.
        // SUGGESTION: Build a QHash<int, LocationState*> mapping location id to pointer before this loop, then set edge.toLocation in O(1) per edge.
        for (auto& loc : state.locations) {
            for (auto& edge : loc.outgoingEdges) {
                if (edge.def) {
                    int toId = edge.def->toLocation;
                    for (auto& targetLoc : state.locations) {
                        if (targetLoc.def && targetLoc.def->id == toId) {
                            edge.toLocation = &targetLoc;
                            break;
                        }
                    }
                }
            }
        }
        result.state = state;
        return result;
    }

private:
    const domain::GameDef& m_gameDef;
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
