#ifndef TGE_PLAYER_RUNTIME_ENGINE_H
#define TGE_PLAYER_RUNTIME_ENGINE_H

#include "../../domain.h"
#include "../types.h"
#include "gameinitializer.h"
#include <optional>

namespace tge {
namespace player {
namespace runtime {

struct CurrentLocation {
    const LocationState* location = nullptr; // Pointer to current dynamic location
    QString description; // Description to show
    QVector<const EdgeState*> options; // Possible outgoing edges
    QVector<QString> debugMessages; // Debug info about what happened
};

struct CurrentTransition {
    const EdgeState* edge = nullptr; // Pointer to the chosen edge
    const LocationState* nextLocation = nullptr; // Pointer to the next location
    QString transitionText;
    QVector<QString> debugMessages;
};

struct FinishLocation {
    const LocationState* location = nullptr;
    QString description;
    QVector<QString> debugMessages;
};

class Engine {
public:
    Engine(const domain::GameDef& gameDef, GameMode mode = GameMode::Normal)
        : m_gameDef(gameDef), m_initializer(gameDef, mode) {
        GameInitResult result = m_initializer.initialize();
        if (result.state.has_value()) {
            m_state = std::move(result.state.value());
        } else {
            m_error = result.error;
        }
    }

    bool hasError() const { return m_error.has_value(); }
    QString error() const { return m_error.value_or(""); }

    std::optional<CurrentLocation> start() {
        if (!m_state.startLocation) return std::nullopt;
        CurrentLocation result;
        result.location = m_state.startLocation;
        if (m_state.startLocation->def) {
            result.description = m_state.startLocation->def->description;
        } else {
            result.description = "";
            result.debugMessages.append("No location definition available.");
        }
        return result;
    }

    // Given a CurrentLocation and the game state, return available transitions
    // TODO

    // Given a CurrentLocation and an edge id, return the transition info
    std::optional<CurrentTransition> choose(const CurrentLocation& current, int edgeId) {
        // Edge not found
        CurrentTransition result;
        result.edge = nullptr;
        result.nextLocation = nullptr;
        result.transitionText = "";
        result.debugMessages.append(QString("Edge id %1 not found among options.").arg(edgeId));
        return result;
    }

    // Given a CurrentTransition, return the next CurrentLocation or FinishLocation
    std::variant<CurrentLocation, FinishLocation> step(const CurrentTransition& transition) {
        throw std::logic_error("Not implemented");
    }

private:
    const domain::GameDef& m_gameDef;
    GameInitializer m_initializer;
    GameState m_state;
    std::optional<QString> m_error;
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_ENGINE_H
