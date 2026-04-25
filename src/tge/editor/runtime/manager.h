#ifndef TGE_EDITOR_RUNTIME_MANAGER_H
#define TGE_EDITOR_RUNTIME_MANAGER_H

#include "../types.h"
#include "../../domain.h"
#include <algorithm>
#include <stdexcept>

namespace tge {
namespace editor {
namespace runtime {

class Manager {
public:
    Manager(domain::GameDef& game, EditorState& state, IdGenerator& idGen)
        : m_game(game), m_state(state), m_idGen(idGen), m_lastError() {}

    // Error handling
    const QString& lastError() const { return m_lastError; }
    void clearError() { m_lastError.clear(); }
    bool hasError() const { return !m_lastError.isEmpty(); }

    // Add a new regular location to the game
    domain::LocationDef& addLocation(const QString& label, int color, int x, int y) {
        domain::LocationDef loc;
        loc.id = m_idGen.generateLocationId();
        loc.type = domain::LocationType::Regular;
        loc.label = label;
        loc.color = color;
        loc.coordX = x;
        loc.coordY = y;
        m_game.locations[loc.id] = loc;
        return m_game.locations[loc.id];
    }

    // Add a new start location (no custom color)
    domain::LocationDef& addStartLocation(const QString& label, int x, int y) {
        domain::LocationDef loc;
        loc.id = m_idGen.generateLocationId();
        loc.type = domain::LocationType::Start;
        loc.label = label;
        loc.color = tge::domain::LOCATION_COLOR_NONE;
        loc.coordX = x;
        loc.coordY = y;
        m_game.locations[loc.id] = loc;
        return m_game.locations[loc.id];
    }

    // Add a new finish location (no custom color)
    domain::LocationDef& addFinishLocation(const QString& label, int x, int y) {
        domain::LocationDef loc;
        loc.id = m_idGen.generateLocationId();
        loc.type = domain::LocationType::Finish;
        loc.label = label;
        loc.color = tge::domain::LOCATION_COLOR_NONE;
        loc.coordX = x;
        loc.coordY = y;
        m_game.locations[loc.id] = loc;
        return m_game.locations[loc.id];
    }

    // Add a new service location (may have custom color)
    domain::LocationDef& addServiceLocation(const QString& label, int color, int x, int y) {
        domain::LocationDef loc;
        loc.id = m_idGen.generateLocationId();
        loc.type = domain::LocationType::Service;
        loc.label = label;
        loc.color = color;
        loc.coordX = x;
        loc.coordY = y;
        m_game.locations[loc.id] = loc;
        return m_game.locations[loc.id];
    }

    // Delete a location by id
    void deleteLocation(int id) {
        if (!m_game.locations.contains(id)) return;

        QVector<int> toRemove;
        for (auto it = m_game.edges.constBegin(); it != m_game.edges.constEnd(); ++it) {
            if (it.value().fromLocation == id || it.value().toLocation == id) {
                toRemove.append(it.key());
            }
        }
        for (int edgeId : toRemove) {
            deleteEdge(edgeId);
        }
        m_game.locations.remove(id);
    }

    // Add an edge between two locations
    domain::EdgeDef* addEdge(int fromId, int toId, const QString& optionText, const QString& transitionText,
                            const QString& condition = "") {
        clearError();
        if (!m_game.locations.contains(fromId) || !m_game.locations.contains(toId)) {
            m_lastError = "Source or destination location does not exist.";
            return nullptr;
        }
        // Prohibit self-pointing edges (except via addLoopEdge)
        if (fromId == toId) {
            m_lastError = "Self-pointing edges are not allowed. Use addLoopEdge instead.";
            return nullptr;
        }
        // Prohibit edges to the start location
        if (m_game.locations[toId].type == domain::LocationType::Start) {
            m_lastError = "Edges to the start location are not allowed.";
            return nullptr;
        }
        // Prohibit outgoing edges from finish locations
        if (m_game.locations[fromId].type == domain::LocationType::Finish) {
            m_lastError = "Outgoing edges from finish locations are not allowed.";
            return nullptr;
        }
        domain::EdgeDef edge;
        edge.id = m_idGen.generateEdgeId();
        edge.fromLocation = fromId;
        edge.toLocation = toId;
        edge.optionText = optionText;
        edge.transitionText = transitionText;
        edge.condition = condition;
        edge.color = tge::domain::LOCATION_COLOR_NONE;
        m_game.edges[edge.id] = edge;

        auto& out = m_game.locations[fromId].outgoingEdges;
        if (std::find(out.begin(), out.end(), edge.id) == out.end()) {
            out.append(edge.id);
        }
        auto& in = m_game.locations[toId].incomingEdges;
        if (std::find(in.begin(), in.end(), edge.id) == in.end()) {
            in.append(edge.id);
        }

        return &m_game.edges[edge.id];
      }

    bool connectEdge(int fromId, int toId, int edgeId) {
        clearError();
        if (!m_game.edges.contains(edgeId)) {
            m_lastError = "Edge ID does not exist.";
            return false;
        }
        if (!m_game.locations.contains(fromId) || !m_game.locations.contains(toId)) {
            m_lastError = "Source or destination location does not exist.";
            return false;
        }
        if (fromId == toId) {
            m_lastError = "Self-pointing edges are not allowed. Use addLoopEdge instead.";
            return false;
        }
        if (m_game.locations[toId].type == domain::LocationType::Start) {
            m_lastError = "Edges to the start location are not allowed.";
            return false;
        }
        if (m_game.locations[fromId].type == domain::LocationType::Finish) {
            m_lastError = "Outgoing edges from finish locations are not allowed.";
            return false;
        }

        const int prevFrom = m_game.edges[edgeId].fromLocation;
        const int prevTo = m_game.edges[edgeId].toLocation;
        if (m_game.locations.contains(prevFrom)) {
            auto& prevOut = m_game.locations[prevFrom].outgoingEdges;
            prevOut.erase(std::remove(prevOut.begin(), prevOut.end(), edgeId), prevOut.end());
        }
        if (m_game.locations.contains(prevTo)) {
            auto& prevIn = m_game.locations[prevTo].incomingEdges;
            prevIn.erase(std::remove(prevIn.begin(), prevIn.end(), edgeId), prevIn.end());
        }

        m_game.edges[edgeId].fromLocation = fromId;
        m_game.edges[edgeId].toLocation = toId;

        // Update outgoing edges of fromId
        auto& out = m_game.locations[fromId].outgoingEdges;
        if (std::find(out.begin(), out.end(), edgeId) == out.end()) {
            out.append(edgeId);
        }
        // Update incoming edges of toId
        auto& in = m_game.locations[toId].incomingEdges;
        if (std::find(in.begin(), in.end(), edgeId) == in.end()) {
            in.append(edgeId);
        }

        return true;
    }

    // Add a loop path by creating a nearby service location and two edges
    // (source->service, service->source).
    // Returns the id of the new service location, or -1 on error
    int addLoopEdge(int sourceId, const QString& optionText, const QString& transitionText,
                    const QString& condition = "") {
        clearError();
        if (!m_game.locations.contains(sourceId)) {
            m_lastError = "Source location does not exist.";
            return -1;
        }
        auto& src = m_game.locations[sourceId];
        // Prohibit for start/finish locations
        if (src.type == domain::LocationType::Start || src.type == domain::LocationType::Finish) {
            m_lastError = "Loop edges for start or finish locations are not allowed.";
            return -1;
        }

        // Place the service location near the source, preferring free adjacent cells.
        QVector<QPair<int, int>> offsets = {
            {1, 0}, {1, 1}, {0, 1}, {-1, 1},
            {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
        };
        int newX = src.coordX + 1;
        int newY = src.coordY;
        for (const auto& off : offsets) {
            int candidateX = src.coordX + off.first;
            int candidateY = src.coordY + off.second;
            bool occupied = false;
            for (auto it = m_game.locations.constBegin(); it != m_game.locations.constEnd(); ++it) {
                if (it.value().coordX == candidateX && it.value().coordY == candidateY) {
                    occupied = true;
                    break;
                }
            }
            if (!occupied) {
                newX = candidateX;
                newY = candidateY;
                break;
            }
        }

        // Create service location
        auto& serviceLoc = addServiceLocation("", tge::domain::LOCATION_COLOR_NONE, newX, newY);

        // Add edge source -> service
        auto* forward = addEdge(sourceId, serviceLoc.id, optionText, transitionText, condition);
        if (!forward) {
            m_game.locations.remove(serviceLoc.id);
            return -1;
        }

        // Add edge service -> source to complete the loop path.
        auto* back = addEdge(serviceLoc.id, sourceId, "", "", "");
        if (!back) {
            deleteEdge(forward->id);
            m_game.locations.remove(serviceLoc.id);
            return -1;
        }

        return serviceLoc.id;
    }

    // Delete an edge by id
    void deleteEdge(int edgeId) {
        if (!m_game.edges.contains(edgeId)) return;
        int fromId = m_game.edges[edgeId].fromLocation;
        if (m_game.locations.contains(fromId)) {
            auto& out = m_game.locations[fromId].outgoingEdges;
            out.erase(std::remove(out.begin(), out.end(), edgeId), out.end());
        }
        int toId = m_game.edges[edgeId].toLocation;
        if (m_game.locations.contains(toId)) {
            auto& in = m_game.locations[toId].incomingEdges;
            in.erase(std::remove(in.begin(), in.end(), edgeId), in.end());
        }
        m_game.edges.remove(edgeId);
    }

    // --- Static helpers ---
    static domain::LocationDef& getLocation(int locationId, domain::GameDef& game) {
        auto it = game.locations.find(locationId);
        if (it == game.locations.end()) {
            throw std::out_of_range("Location ID not found");
        }
        return it.value();
    }
    static domain::EdgeDef& getEdge(int edgeId, domain::GameDef& game) {
        auto it = game.edges.find(edgeId);
        if (it == game.edges.end()) {
            throw std::out_of_range("Edge ID not found");
        }
        return it.value();
    }
    static std::pair<domain::LocationDef&, domain::LocationDef&> getEdgeLocations(int edgeId, domain::GameDef& game) {
        auto edgeIt = game.edges.find(edgeId);
        if (edgeIt == game.edges.end()) {
            throw std::out_of_range("Edge ID not found");
        }
        auto fromIt = game.locations.find(edgeIt.value().fromLocation);
        auto toIt = game.locations.find(edgeIt.value().toLocation);
        if (fromIt == game.locations.end() || toIt == game.locations.end()) {
            throw std::out_of_range("Edge location ID not found");
        }
        return { fromIt.value(), toIt.value() };
    }

    domain::GameDef& game() { return m_game; }
    const domain::GameDef& game() const { return m_game; }

private:
    domain::GameDef& m_game;
    EditorState& m_state;
    IdGenerator& m_idGen;
    QString m_lastError;
};

} // namespace runtime
} // namespace editor
} // namespace tge

#endif // TGE_EDITOR_RUNTIME_MANAGER_H
