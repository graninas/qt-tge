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
    Engine(const domain::GameDef& gameDef)
        : m_gameDef(gameDef), m_initializer(gameDef) {
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
        // Get the first description from the pack (no selector logic yet)
        if (m_state.startLocation->def && !m_state.startLocation->def->descriptionPack.descriptions.isEmpty()) {
            result.description = m_state.startLocation->def->descriptionPack.descriptions[0];
            result.debugMessages.append("Selector: not implemented, picked first description.");
        } else {
            result.description = "";
            result.debugMessages.append("No descriptions available.");
        }
        // Add outgoing edges as options
        for (const auto* edge : m_state.startLocation->outgoingEdges) {
            result.options.append(edge);
        }
        return result;
    }

    // Given a CurrentLocation and an edge id, return the transition info
    std::optional<CurrentTransition> choose(const CurrentLocation& current, int edgeId) {
        for (const EdgeState* edge : current.options) {
            if (edge && edge->def && edge->def->id == edgeId) {
                CurrentTransition result;
                result.edge = edge;
                result.transitionText = edge->def->transitionText;
                result.nextLocation = edge->toLocation;
                result.debugMessages.append(QString("Transition via edge id %1: '%2'")
                    .arg(edgeId)
                    .arg(edge->def->transitionText));
                if (result.nextLocation) {
                    result.debugMessages.append(QString("Next location id: %1").arg(result.nextLocation->def->id));
                } else {
                    result.debugMessages.append("Next location not found.");
                }
                return result;
            }
        }
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
        if (!transition.nextLocation) {
            FinishLocation finish;
            finish.location = nullptr;
            finish.description = "No next location (end of path).";
            finish.debugMessages = transition.debugMessages;
            finish.debugMessages.append("No next location pointer in transition.");
            return finish;
        }
        const LocationState* loc = transition.nextLocation;
        if (!loc->def) {
            FinishLocation finish;
            finish.location = loc;
            finish.description = "No static definition for next location.";
            finish.debugMessages = transition.debugMessages;
            finish.debugMessages.append("No static definition for next location.");
            return finish;
        }
        if (loc->def->type == domain::LocationType::Finish) {
            FinishLocation finish;
            finish.location = loc;
            if (!loc->def->descriptionPack.descriptions.isEmpty())
                finish.description = loc->def->descriptionPack.descriptions[0];
            else
                finish.description = "";
            finish.debugMessages = transition.debugMessages;
            finish.debugMessages.append("Arrived at finish location.");
            return finish;
        } else {
            CurrentLocation next;
            next.location = loc;
            if (!loc->def->descriptionPack.descriptions.isEmpty()) {
                next.description = loc->def->descriptionPack.descriptions[0];
                next.debugMessages = transition.debugMessages;
                next.debugMessages.append("Selector: not implemented, picked first description.");
            } else {
                next.description = "";
                next.debugMessages = transition.debugMessages;
                next.debugMessages.append("No descriptions available.");
            }
            for (const auto* edge : loc->outgoingEdges) {
                next.options.append(edge);
            }
            return next;
        }
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
