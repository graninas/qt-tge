#ifndef TGE_PLAYER_RUNTIME_ENGINE_H
#define TGE_PLAYER_RUNTIME_ENGINE_H

#include "../../domain.h"
#include "../../formula/translator.h"
#include "../types.h"
#include "gameinitializer.h"
#include <map>
#include <optional>
#include <stdexcept>
#include <variant>

namespace tge {
namespace player {
namespace runtime {

struct TransitionOption {
    const EdgeState* edge = nullptr;
    bool isAvailable = false;
    QVector<QString> debugMessages;
};

struct PendingVariableChange {
    int variableIndex = 0;
    const VariableDef* def = nullptr;
    QString newValue;
};

struct PendingInfoDisplayItemChange
{
  int itemIndex = 0;
  const domain::InfoDisplayItemDef *def = nullptr;
  bool changePriority = false;
  int newPriority = 0;
  bool changeVisibility = false;
  bool newVisibility = false;
  bool changeShowValue = false;
  bool newShowValue = false;
  bool changeValue = false;
  std::string newValue;
};

struct CurrentLocation {
    const LocationState* location = nullptr; // Pointer to current dynamic location
    QString description; // Description to show
    QVector<TransitionOption> options; // Possible outgoing edges and their current availability
    QVector<QString> debugMessages; // Debug info about what happened
};

struct CurrentTransition {
    const EdgeState* edge = nullptr; // Pointer to the chosen edge
    const LocationState* nextLocation = nullptr; // Pointer to the next location
    QString transitionText;
    QVector<PendingVariableChange> pendingVariableChanges;
    QVector<PendingInfoDisplayItemChange> pendingInfoDisplayItemChanges;
    QVector<QString> debugMessages;
};

struct FinishLocation {
    const LocationState* location = nullptr;
    QString description;
    QVector<QString> debugMessages;
};

using StepResult = std::variant<CurrentLocation, FinishLocation, CurrentTransition>;

class Engine {
public:
    Engine(const domain::GameDef& gameDef, GameMode mode = GameMode::Normal)
        : m_gameDef(gameDef), m_initializer(gameDef, mode) {
        GameInitResult result = m_initializer.initialize();
        if (result.state.has_value()) {
            m_state = std::move(result.state.value());
            const QString infoInitError = initializeInfoDisplayValues();
            if (!infoInitError.isEmpty()) {
                m_error = infoInitError;
            }
        } else {
            m_error = result.error;
        }
    }

    bool hasError() const { return m_error.has_value(); }
    QString error() const { return m_error.value_or(""); }
    const GameState& state() const { return m_state; }

    std::optional<CurrentLocation> start() {
        if (!m_state.startLocation) return std::nullopt;
        return buildCurrentLocation(m_state.startLocation);
    }

    // Given a CurrentLocation and an edge id, return the transition info
    std::optional<CurrentTransition> choose(const CurrentLocation& current, int edgeId) {
        if (!current.location || !current.location->def) {
            return std::nullopt;
        }

        const TransitionOption* selected = nullptr;
        for (const auto& option : current.options) {
            if (option.edge && option.edge->def && option.edge->def->id == edgeId) {
                selected = &option;
                break;
            }
        }

        if (!selected || !selected->edge || !selected->edge->def) {
            return std::nullopt;
        }
        if (!selected->isAvailable) {
            return std::nullopt;
        }

        CurrentTransition result;
        result.edge = selected->edge;
        result.transitionText = selected->edge->def->transitionText;
        result.debugMessages = selected->debugMessages;

        const auto nextLocationIt = m_state.locations.find(selected->edge->def->toLocation);
        if (nextLocationIt == m_state.locations.end()) {
            result.debugMessages.append(QString("Edge id %1 points to unknown location %2.")
                                            .arg(edgeId)
                                            .arg(selected->edge->def->toLocation));
            return std::nullopt;
        }
        result.nextLocation = nextLocationIt->second.get();

        for (const auto& setting : selected->edge->def->variableSettings) {
            QString conditionError;
            if (!evaluateCondition(setting.edgeVariableCondition, &conditionError)) {
                if (!conditionError.isEmpty()) {
                    result.debugMessages.append(QString("Variable setting P%1 condition error: %2")
                                                    .arg(setting.variableIndex)
                                                    .arg(conditionError));
                }
                continue;
            }

            VariableState* variableState = findVariableState(setting.variableIndex);
            if (!variableState || !variableState->def) {
                result.debugMessages.append(QString("Variable setting references unknown parameter P%1.")
                                                .arg(setting.variableIndex));
                continue;
            }
            if (setting.newValueFormula.trimmed().isEmpty()) {
                continue;
            }

            QString formulaError;
            const QString newValue = evaluateValueFormula(setting.newValueFormula, &formulaError);
            if (!formulaError.isEmpty()) {
                result.debugMessages.append(QString("Variable setting P%1 evaluation error: %2")
                                                .arg(setting.variableIndex)
                                                .arg(formulaError));
                continue;
            }

            PendingVariableChange change;
            change.variableIndex = setting.variableIndex;
            change.def = variableState->def;
            change.newValue = newValue;
            result.pendingVariableChanges.append(change);
        }

        for (const auto& setting : selected->edge->def->infoDisplayItemSettings) {
            InfoDisplayItemState* itemState = findInfoDisplayItemState(setting.itemIndex);
            if (!itemState || !itemState->def) {
                result.debugMessages.append(QString("Info display setting references unknown item %1.")
                                                .arg(setting.itemIndex));
                continue;
            }

            PendingInfoDisplayItemChange change;
            change.itemIndex = setting.itemIndex;
            change.def = itemState->def;
            change.changePriority = setting.changePriority;
            change.newPriority = setting.newPriority;
            change.changeVisibility = setting.changeVisibility && itemState->allowVisibilityChanges;
            change.newVisibility = setting.newVisibility;
            change.changeShowValue = setting.changeShowValue;
            change.newShowValue = setting.newShowValue;

            if (setting.changeVisibility && !itemState->allowVisibilityChanges) {
                result.debugMessages.append(QString("Visibility change for info item %1 ignored by current game mode.")
                                                .arg(setting.itemIndex));
            }

            result.pendingInfoDisplayItemChanges.append(change);
        }

        appendFormulaDrivenInfoDisplayPendingChanges(result.pendingVariableChanges,
                                                    result.pendingInfoDisplayItemChanges,
                                                    result.debugMessages);

        return result;
    }

    // Given a CurrentTransition, return the next CurrentLocation or FinishLocation
    StepResult step(const CurrentTransition& transition) {
        for (const auto& change : transition.pendingVariableChanges) {
            VariableState* variableState = findVariableState(change.variableIndex);
            if (variableState) {
                variableState->value = change.newValue;
            }
        }

        for (const auto& change : transition.pendingInfoDisplayItemChanges) {
            InfoDisplayItemState* itemState = findInfoDisplayItemState(change.itemIndex);
            if (!itemState) {
                continue;
            }
            if (change.changePriority) {
                itemState->priority = change.newPriority;
            }
            if (change.changeVisibility && itemState->allowVisibilityChanges) {
                itemState->visible = change.newVisibility;
            }
            if (change.changeShowValue) {
                itemState->showFormulaValue = change.newShowValue;
            }
            if (change.changeValue) {
                itemState->value = change.newValue;
            }
        }

        if (!transition.nextLocation || !transition.nextLocation->def) {
            CurrentLocation invalidResult;
            invalidResult.debugMessages = transition.debugMessages;
            invalidResult.debugMessages.append("Transition has no next location definition.");
            return invalidResult;
        }

        if (transition.nextLocation->def->type == domain::LocationType::Finish) {
            FinishLocation result;
            result.location = transition.nextLocation;
            result.description = transition.nextLocation->def->description;
            result.debugMessages = transition.debugMessages;
            return result;
        }

        CurrentLocation next = buildCurrentLocation(transition.nextLocation, transition.debugMessages);
        if (transition.nextLocation->def->type == domain::LocationType::Service) {
            const TransitionOption* automaticOption = nullptr;
            int availableCount = 0;
            for (const auto& option : next.options) {
                if (option.isAvailable) {
                    automaticOption = &option;
                    ++availableCount;
                }
            }

            if (availableCount == 1 && automaticOption && automaticOption->edge && automaticOption->edge->def) {
                std::optional<CurrentTransition> automaticTransition = choose(next, automaticOption->edge->def->id);
                if (automaticTransition.has_value()) {
                    automaticTransition->debugMessages.append(
                        QString("Automatic service transition %1 selected.").arg(automaticOption->edge->def->id));
                    return automaticTransition.value();
                }
            }
        }

        return next;
    }

private:
    std::map<std::string, int> buildFormulaParams(const QVector<PendingVariableChange>* pendingVariableChanges = nullptr) const {
        std::map<std::string, int> params;
        for (const auto& variable : m_state.variables) {
            if (!variable.def) {
                continue;
            }
            bool ok = false;
            const int value = variable.value.toInt(&ok);
            if (!ok) {
                throw std::runtime_error(
                    QString("Variable %1 has non-integer value '%2'.")
                        .arg(variable.def->index)
                        .arg(variable.value)
                        .toStdString());
            }
            params.emplace(variable.def->index.toStdString(), value);
        }
        if (pendingVariableChanges) {
            for (const auto& change : *pendingVariableChanges) {
                if (!change.def) {
                    continue;
                }
                bool ok = false;
                const int value = change.newValue.toInt(&ok);
                if (!ok) {
                    throw std::runtime_error(
                        QString("Pending variable %1 has non-integer value '%2'.")
                            .arg(change.def->index)
                            .arg(change.newValue)
                            .toStdString());
                }
                params[change.def->index.toStdString()] = value;
            }
        }
        return params;
    }

    bool evaluateCondition(const QString& formulaText, QString* error = nullptr) const {
        const QString trimmed = formulaText.trimmed();
        if (trimmed.isEmpty()) {
            return true;
        }
        try {
            return tge::formula::parseAndEvaluateExpression(trimmed.toStdString(), buildFormulaParams()) != 0;
        } catch (const std::exception& exception) {
            if (error) {
                *error = QString::fromStdString(exception.what());
            }
            return false;
        }
    }

    QString evaluateValueFormula(const QString& formulaText, QString* error = nullptr) const {
        return evaluateValueFormula(formulaText, buildFormulaParams(), error);
    }

    QString evaluateValueFormula(const QString& formulaText,
                                 const std::map<std::string, int>& params,
                                 QString* error = nullptr) const {
        const QString trimmed = formulaText.trimmed();
        if (trimmed.isEmpty()) {
            return "";
        }
        try {
            const int value = tge::formula::parseAndEvaluateExpression(trimmed.toStdString(), params);
            return QString::number(value);
        } catch (const std::exception& exception) {
            if (error) {
                *error = QString::fromStdString(exception.what());
            }
            return "";
        }
    }

    VariableState* findVariableState(int variableIndex) {
        const QString expectedIndex = QString("P%1").arg(variableIndex);
        for (auto& variable : m_state.variables) {
            if (variable.def && variable.def->index == expectedIndex) {
                return &variable;
            }
        }
        return nullptr;
    }

    InfoDisplayItemState* findInfoDisplayItemState(int itemIndex) {
        for (auto& item : m_state.infoDisplayItems) {
            if (item.def && item.def->id == itemIndex) {
                return &item;
            }
        }
        return nullptr;
    }

    PendingInfoDisplayItemChange* findPendingInfoDisplayItemChange(
        QVector<PendingInfoDisplayItemChange>& pendingChanges,
        int itemIndex) const {
        for (auto& change : pendingChanges) {
            if (change.itemIndex == itemIndex) {
                return &change;
            }
        }
        return nullptr;
    }

    void appendFormulaDrivenInfoDisplayPendingChanges(
        const QVector<PendingVariableChange>& pendingVariableChanges,
        QVector<PendingInfoDisplayItemChange>& pendingItemChanges,
        QVector<QString>& debugMessages) const {
        std::map<std::string, int> projectedParams;
        try {
            projectedParams = buildFormulaParams(&pendingVariableChanges);
        } catch (const std::exception& exception) {
            debugMessages.append(QString("Failed to prepare projected HUD values: %1")
                                     .arg(QString::fromStdString(exception.what())));
            return;
        }

        for (const auto& item : m_state.infoDisplayItems) {
            if (!item.def) {
                continue;
            }

            QString formulaError;
            const QString projectedValue = evaluateValueFormula(item.def->valueFormula, projectedParams, &formulaError);
            if (!formulaError.isEmpty()) {
                debugMessages.append(QString("Info item %1 formula recalculation error: %2")
                                         .arg(item.def->id)
                                         .arg(formulaError));
                continue;
            }

            if (projectedValue.toStdString() == item.value) {
                continue;
            }

            PendingInfoDisplayItemChange* pendingChange =
                findPendingInfoDisplayItemChange(pendingItemChanges, item.def->id);
            if (!pendingChange) {
                PendingInfoDisplayItemChange newChange;
                newChange.itemIndex = item.def->id;
                newChange.def = item.def;
                pendingItemChanges.append(newChange);
                pendingChange = &pendingItemChanges.last();
            }

            pendingChange->changeValue = true;
            pendingChange->newValue = projectedValue.toStdString();
        }
    }

    QString initializeInfoDisplayValues() {
        for (auto& item : m_state.infoDisplayItems) {
            if (!item.def) {
                continue;
            }
            QString formulaError;
            item.value = evaluateValueFormula(item.def->valueFormula, &formulaError).toStdString();
            if (!formulaError.isEmpty()) {
                return QString("Failed to initialize info item %1: %2")
                    .arg(item.def->id)
                    .arg(formulaError);
            }
        }
        return "";
    }

    CurrentLocation buildCurrentLocation(const LocationState* location, const QVector<QString>& inheritedMessages = {}) const {
        CurrentLocation result;
        result.location = location;
        result.debugMessages = inheritedMessages;

        if (!location || !location->def) {
            result.description = "";
            result.debugMessages.append("No location definition available.");
            return result;
        }

        result.description = location->def->description;
        result.options.reserve(location->def->outgoingEdges.size());
        for (int edgeId : location->def->outgoingEdges) {
            TransitionOption option;

            const auto edgeIt = m_state.edges.find(edgeId);
            if (edgeIt == m_state.edges.end() || !edgeIt->second || !edgeIt->second->def) {
                option.isAvailable = false;
                option.debugMessages.append(QString("Outgoing edge %1 is missing from runtime state.").arg(edgeId));
                result.options.append(option);
                continue;
            }

            option.edge = edgeIt->second.get();
            option.isAvailable = true;

            QString conditionError;
            if (!evaluateCondition(option.edge->def->condition, &conditionError)) {
                option.isAvailable = false;
                if (!conditionError.isEmpty()) {
                    option.debugMessages.append(QString("Edge condition error: %1").arg(conditionError));
                }
            }

            for (const auto& setting : option.edge->def->variableSettings) {
                if (!option.isAvailable) {
                    break;
                }
                if (!evaluateCondition(setting.edgeVariableCondition, &conditionError)) {
                    option.isAvailable = false;
                    if (!conditionError.isEmpty()) {
                        option.debugMessages.append(
                            QString("Variable condition for P%1 error: %2")
                                .arg(setting.variableIndex)
                                .arg(conditionError));
                    }
                }
            }

            result.options.append(option);
        }

        return result;
    }

    const domain::GameDef& m_gameDef;
    GameInitializer m_initializer;
    GameState m_state;
    std::optional<QString> m_error;
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_ENGINE_H
