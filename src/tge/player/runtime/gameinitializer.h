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
        if (m_gameDef.locations.empty()) {
            result.error = QString("GameDef must have at least one location");
            return result;
        }
        GameState state;
        state.startLocation = nullptr;

        // Step 1: Create all LocationState objects
        for (auto it = m_gameDef.locations.begin(); it != m_gameDef.locations.end(); ++it) {
            const auto& locDef = it.value();
            auto locState = std::make_unique<LocationState>();
            locState->def = &locDef;
            for (const auto& varDef : locDef.localVariables) {
                VariableState varState;
                varState.def = &varDef;
                varState.value = "";
                locState->localVariables.push_back(varState);
            }
            int locId = locDef.id;
            if (!state.startLocation && locDef.type == domain::LocationType::Start) {
                state.startLocation = locState.get();
            }
            state.locations.emplace(locId, std::move(locState));
        }

        // Step 2: Create all EdgeState objects
        for (auto it = m_gameDef.edges.begin(); it != m_gameDef.edges.end(); ++it) {
            const auto& edgeDef = it.value();
            auto edgeState = std::make_unique<EdgeState>();
            edgeState->def = &edgeDef;
            int edgeId = edgeDef.id;
            state.edges.emplace(edgeId, std::move(edgeState));
        }

        result.state = std::move(state);
        return result;
    }

private:
    const domain::GameDef& m_gameDef;
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
