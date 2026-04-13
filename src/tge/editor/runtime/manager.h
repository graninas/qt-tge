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
        : m_game(game), m_state(state), m_idGen(idGen) {}

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
    domain::EdgeDef& addEdge(int fromId, int toId, const QString& optionText, const QString& transitionText) {
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
        return m_game.edges[edge.id];
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

private:
    domain::GameDef& m_game;
    EditorState& m_state;
    IdGenerator& m_idGen;
};

} // namespace runtime
} // namespace editor
} // namespace tge

#endif // TGE_EDITOR_RUNTIME_MANAGER_H
