#ifndef TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
#define TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H

#include <QString>
#include <optional>
#include "../../domain.h"
#include "../../formula/parser.h"
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
    GameInitializer(const domain::GameDef& gameDef, GameMode mode)
        : m_gameDef(gameDef), m_mode(mode) {}

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
            if (it.key() != it.value().id) {
                result.error = QString("Location map key %1 does not match location.id %2")
                                   .arg(it.key())
                                   .arg(it.value().id);
                return result;
            }
            const auto& locDef = it.value();
            auto locState = std::make_unique<LocationState>();
            locState->def = &locDef;
            int locId = locDef.id;
            if (!state.startLocation && locDef.type == domain::LocationType::Start) {
                state.startLocation = locState.get();
            }
            state.locations.emplace(locId, std::move(locState));
        }

        // Step 2: Create all EdgeState objects
        for (auto it = m_gameDef.edges.begin(); it != m_gameDef.edges.end(); ++it) {
            if (it.key() != it.value().id) {
                result.error = QString("Edge map key %1 does not match edge.id %2")
                                   .arg(it.key())
                                   .arg(it.value().id);
                return result;
            }
            const auto& edgeDef = it.value();
            if (m_gameDef.locations.find(edgeDef.fromLocation) == m_gameDef.locations.end()) {
                result.error = QString("Edge %1 has unknown fromLocation %2")
                                   .arg(edgeDef.id)
                                   .arg(edgeDef.fromLocation);
                return result;
            }
            if (m_gameDef.locations.find(edgeDef.toLocation) == m_gameDef.locations.end()) {
                result.error = QString("Edge %1 has unknown toLocation %2")
                                   .arg(edgeDef.id)
                                   .arg(edgeDef.toLocation);
                return result;
            }
            auto edgeState = std::make_unique<EdgeState>();
            edgeState->def = &edgeDef;
            edgeState->conditionFormula = parseFormula(edgeDef.condition);
            edgeState->variableSettings.reserve(static_cast<size_t>(edgeDef.variableSettings.size()));
            for (const auto& settingDef : edgeDef.variableSettings) {
                EdgeVariableSettingState settingState;
                settingState.def = &settingDef;
                settingState.conditionFormula = parseFormula(settingDef.edgeVariableCondition);
                settingState.newValueFormula = parseFormula(settingDef.newValueFormula);
                edgeState->variableSettings.push_back(std::move(settingState));
            }
            int edgeId = edgeDef.id;
            state.edges.emplace(edgeId, std::move(edgeState));
        }

        // Step 3: Initialize global variables
        state.variables.reserve(static_cast<size_t>(m_gameDef.globalVariables.size()));
        for (const auto& variableDef : m_gameDef.globalVariables) {
            VariableState variableState;
            variableState.def = &variableDef;
            variableState.value = variableDef.defaultValue;
            state.variables.push_back(variableState);
        }

        // Step 4: Initialize info display item states
        state.infoDisplayItems.reserve(static_cast<size_t>(m_gameDef.infoDisplayItems.size()));
        for (const auto& itemDef : m_gameDef.infoDisplayItems) {
            InfoDisplayItemState itemState;
            itemState.def = &itemDef;
            itemState.value = "";
            itemState.priority = itemDef.priority;
            itemState.showFormulaValue = itemDef.showFormulaValue;
            itemState.valueFormula = parseFormula(itemDef.valueFormula);

            if (m_mode == GameMode::Debug) {
                itemState.visible = true;
                itemState.allowVisibilityChanges = false;
            } else {
                if (itemDef.mode == domain::InfoDisplayItemMode::Debug) {
                    itemState.visible = false;
                    itemState.allowVisibilityChanges = false;
                } else {
                    itemState.visible = itemDef.isVisible;
                    itemState.allowVisibilityChanges = true;
                }
            }

            state.infoDisplayItems.push_back(itemState);
        }

        if (!state.startLocation) {
            result.error = QString("GameDef must contain a Start location");
            return result;
        }

        result.state = std::move(state);
        return result;
    }

private:
    static ParsedFormulaState parseFormula(const QString& formulaText) {
        ParsedFormulaState parsed;
        const QString trimmed = formulaText.trimmed();
        if (trimmed.isEmpty()) {
            return parsed;
        }

        const tge::formula::ParseResult result = tge::formula::parse(trimmed.toStdString());
        if (!result.error.empty()) {
            parsed.parseError = QString::fromStdString(result.error);
            return parsed;
        }

        parsed.ast = result.ast;
        return parsed;
    }

    const domain::GameDef& m_gameDef;
    GameMode m_mode;
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
