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
        m_game.locations.append(loc);
        return m_game.locations.last();
    }

    // Delete a location by id
    void deleteLocation(int id) {
        for (int i = 0; i < m_game.locations.size(); ++i) {
            if (m_game.locations[i].id == id) {
                m_game.locations.remove(i);
                break;
            }
        }
        // Optionally: remove edges pointing to/from this location
    }

    // Add an edge between two locations
    domain::EdgeDef& addEdge(int fromId, int toId, const QString& optionText, const QString& transitionText) {
        domain::EdgeDef edge;
        edge.id = m_idGen.generateEdgeId();
        edge.fromLocation = fromId;
        edge.toLocation = toId;
        edge.optionText = optionText;
        edge.transitionText = transitionText;
        for (auto& loc : m_game.locations) {
            if (loc.id == fromId) {
                loc.outgoingEdges.append(edge);
                return loc.outgoingEdges.last();
            }
        }
        throw std::runtime_error("fromId not found");
    }

    // Delete an edge by id from a location
    void deleteEdge(int fromId, int edgeId) {
        for (auto& loc : m_game.locations) {
            if (loc.id == fromId) {
                for (int i = 0; i < loc.outgoingEdges.size(); ++i) {
                    if (loc.outgoingEdges[i].id == edgeId) {
                        loc.outgoingEdges.remove(i);
                        return;
                    }
                }
            }
        }
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
