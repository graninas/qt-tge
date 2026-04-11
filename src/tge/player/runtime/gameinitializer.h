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
        // For each static location, create a LocationState
        for (const auto& locDef : m_gameDef.locations) {
            LocationState locState;
            locState.def = &locDef;
            // For each static local variable, create a VariableState
            for (const auto& varDef : locDef.localVariables) {
                VariableState varState;
                varState.def = &varDef;
                varState.value = ""; // Dummy value for now
                locState.localVariables.append(varState);
            }
            // For each static outgoing edge, create an EdgeState
            for (const auto& edgeDef : locDef.outgoingEdges) {
                EdgeState edgeState;
                edgeState.def = &edgeDef;
                locState.outgoingEdges.append(edgeState);
            }
            state.locations.append(locState);
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
