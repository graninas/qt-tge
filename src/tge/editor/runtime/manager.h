#ifndef TGE_EDITOR_RUNTIME_MANAGER_H
#define TGE_EDITOR_RUNTIME_MANAGER_H

#include "../types.h"
#include "../../domain.h"

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

    // Add a new location to the game
    domain::LocationDef& addLocation(domain::LocationType type, const QString& label, int color, int x, int y) {
        domain::LocationDef loc;
        loc.id = m_idGen.generateLocationId();
        loc.type = type;
        loc.label = label;
        loc.color = color;
        loc.coordX = x;
        loc.coordY = y;
        m_game.locations[loc.id] = loc;
        return m_game.locations[loc.id];
    }

    // Delete a location by id
    void deleteLocation(int id) {
        m_game.locations.remove(id);
        // Optionally: remove edges pointing to/from this location
        // Remove outgoing edges from this location
        QVector<int> toRemove;
        for (auto it = m_game.edges.begin(); it != m_game.edges.end(); ++it) {
            if (it.value().fromLocation == id || it.value().toLocation == id) {
                toRemove.append(it.key());
            }
        }
        for (int edgeId : toRemove) {
            m_game.edges.remove(edgeId);
        }
    }

    // Add an edge between two locations
    domain::EdgeDef* addEdge(int fromId, int toId, const QString& optionText, const QString& transitionText) {
        clearError();
        // Prohibit self-pointing edges (except via addLoopEdge)
        if (fromId == toId) {
            m_lastError = "Self-pointing edges are not allowed. Use addLoopEdge instead.";
            return nullptr;
        }
        // Prohibit edges to the start location
        if (m_game.locations.contains(toId) && m_game.locations[toId].type == domain::LocationType::Start) {
            m_lastError = "Edges to the start location are not allowed.";
            return nullptr;
        }
        // Prohibit self-edges for start and finish locations
        if (fromId == toId && m_game.locations.contains(fromId)) {
            auto type = m_game.locations[fromId].type;
            if (type == domain::LocationType::Start || type == domain::LocationType::Finish) {
                m_lastError = "Self-edges for start or finish locations are not allowed.";
                return nullptr;
            }
        }
        // Prohibit outgoing edges from finish locations
        if (m_game.locations.contains(fromId) && m_game.locations[fromId].type == domain::LocationType::Finish) {
            m_lastError = "Outgoing edges from finish locations are not allowed.";
            return nullptr;
        }
        domain::EdgeDef edge;
        edge.id = m_idGen.generateEdgeId();
        edge.fromLocation = fromId;
        edge.toLocation = toId;
        edge.optionText = optionText;
        edge.transitionText = transitionText;
        m_game.edges[edge.id] = edge;
        // Add edge id to the location's outgoingEdges
        if (m_game.locations.contains(fromId)) {
            m_game.locations[fromId].outgoingEdges.append(edge.id);
        }
        return &m_game.edges[edge.id];
    }

    // Add a loop edge by creating a service location and two edges (source->service, service->source)
    // Returns the id of the new service location, or -1 on error
    int addLoopEdge(int sourceId, const QString& optionText, const QString& transitionText) {
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
        // Prohibit if already a service location
        if (src.type == domain::LocationType::Service) {
            m_lastError = "Loop edges for service locations are not allowed.";
            return -1;
        }
        // Prohibit outgoing edges from finish locations (shouldn't happen, but for safety)
        if (src.type == domain::LocationType::Finish) {
            m_lastError = "Outgoing edges from finish locations are not allowed.";
            return -1;
        }
        // Find a nearby coordinate for the service location
        int dx = 1, dy = 0;
        int newX = src.coordX + dx;
        int newY = src.coordY + dy;
        // Create service location
        auto& serviceLoc = addLocation(domain::LocationType::Service, "", tge::domain::LOCATION_COLOR_NONE, newX, newY);
        // Add edge source -> service
        if (!addEdge(sourceId, serviceLoc.id, optionText, transitionText)) return -1;
        // Add edge service -> source
        if (!addEdge(serviceLoc.id, sourceId, optionText, transitionText)) return -1;
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
        m_game.edges.remove(edgeId);
    }

    // --- Static helpers ---
    static domain::LocationDef& getLocation(int locationId, domain::GameDef& game) {
        return game.locations[locationId];
    }
    static domain::EdgeDef& getEdge(int edgeId, domain::GameDef& game) {
        return game.edges[edgeId];
    }
    static std::pair<domain::LocationDef&, domain::LocationDef&> getEdgeLocations(int edgeId, domain::GameDef& game) {
        auto& edge = game.edges[edgeId];
        return { game.locations[edge.fromLocation], game.locations[edge.toLocation] };
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
