#include "edgedialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QTextEdit>
#include <QVBoxLayout>

#include <functional>

#include "graphwidget_helpers.h"
#include "tge/domain.h"
#include "tge/formula/parser.h"

EdgeDialog::EdgeDialog(const tge::domain::EdgeDef& edge,
                       const tge::domain::LocationDef& fromLoc,
                       const tge::domain::LocationDef& toLoc,
                       const QVector<tge::domain::VariableDef>& globalVariables,
                       QWidget* parent)
    : QDialog(parent)
    , m_colorButtonGroup(nullptr)
    , m_globalVariables(globalVariables)
    , m_variableList(nullptr)
    , m_variableConditionEdit(nullptr)
    , m_variableNewValueEdit(nullptr)
    , m_variableConditionStatusLabel(nullptr)
    , m_variableNewValueStatusLabel(nullptr)
    , m_variableOverallStatusLabel(nullptr)
    , m_selectedColor(edge.color)
{
    for (const auto& setting : edge.variableSettings) {
        m_variableSettingsByIndex.insert(setting.variableIndex, setting);
    }

    setWindowTitle(tr("Edge Info"));
    QVBoxLayout* layout = new QVBoxLayout(this);

    m_idLabel = new QLabel(tr("Edge ID: %1").arg(edge.id), this);
    layout->addWidget(m_idLabel);

    m_fromLabel = new QLabel(tr("From: %1 (%2)").arg(fromLoc.id).arg(fromLoc.label), this);
    layout->addWidget(m_fromLabel);

    m_toLabel = new QLabel(tr("To: %1 (%2)").arg(toLoc.id).arg(toLoc.label), this);
    layout->addWidget(m_toLabel);

    layout->addWidget(new QLabel(tr("Color:"), this));
    QGridLayout* colorGrid = new QGridLayout();
    m_colorButtonGroup = new QButtonGroup(this);
    m_colorButtonGroup->setExclusive(true);
    m_colorButtons.clear();
    for (int i = 0; i < tge::domain::LOCATION_COLOR_COUNT + 1; ++i) {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(28, 28);
        if (i < tge::domain::LOCATION_COLOR_COUNT) {
            btn->setStyleSheet(QString("background-color: %1;").arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            btn->setToolTip(tr("Color %1").arg(i + 1));
        } else {
            btn->setStyleSheet("background: none; border: 1px dashed #aaa;");
            btn->setText("X");
            btn->setToolTip(tr("Default edge color"));
        }
        m_colorButtonGroup->addButton(btn, i);
        m_colorButtons.append(btn);
        colorGrid->addWidget(btn, i / 8, i % 8);
    }
    layout->addLayout(colorGrid);
    connect(m_colorButtonGroup, &QButtonGroup::idClicked, this, &EdgeDialog::onColorButtonClicked);
    updateColorSelection();

    layout->addWidget(new QLabel(tr("Option Text:"), this));
    m_optionEdit = new QTextEdit(edge.optionText, this);
    m_optionEdit->setMaximumHeight(50);
    layout->addWidget(m_optionEdit);

    layout->addWidget(new QLabel(tr("Transition Text:"), this));
    m_transitionEdit = new QTextEdit(edge.transitionText, this);
    layout->addWidget(m_transitionEdit);

    layout->addWidget(new QLabel(tr("Condition Formula:"), this));
    m_conditionEdit = new QTextEdit(edge.condition, this);
    m_conditionEdit->setMaximumHeight(60);
    layout->addWidget(m_conditionEdit);

    m_conditionStatusLabel = new QLabel(this);
    layout->addWidget(m_conditionStatusLabel);

    QGroupBox* variableGroup = new QGroupBox(tr("Global Variable Settings"), this);
    QHBoxLayout* variableRoot = new QHBoxLayout(variableGroup);

    m_variableList = new QListWidget(variableGroup);
    variableRoot->addWidget(m_variableList, 2);

    QVBoxLayout* variableEditorLayout = new QVBoxLayout();
    variableEditorLayout->addWidget(new QLabel(tr("Additional Condition Formula:"), variableGroup));
    m_variableConditionEdit = new QTextEdit(variableGroup);
    m_variableConditionEdit->setMaximumHeight(55);
    variableEditorLayout->addWidget(m_variableConditionEdit);

    m_variableConditionStatusLabel = new QLabel(variableGroup);
    variableEditorLayout->addWidget(m_variableConditionStatusLabel);

    variableEditorLayout->addWidget(new QLabel(tr("New Value Formula:"), variableGroup));
    m_variableNewValueEdit = new QTextEdit(variableGroup);
    m_variableNewValueEdit->setMaximumHeight(55);
    variableEditorLayout->addWidget(m_variableNewValueEdit);

    m_variableNewValueStatusLabel = new QLabel(variableGroup);
    variableEditorLayout->addWidget(m_variableNewValueStatusLabel);

    m_variableOverallStatusLabel = new QLabel(variableGroup);
    variableEditorLayout->addWidget(m_variableOverallStatusLabel);
    variableEditorLayout->addStretch(1);

    variableRoot->addLayout(variableEditorLayout, 3);
    layout->addWidget(variableGroup);

    rebuildVariableList();
    if (m_globalVariables.isEmpty()) {
        onVariableSelectionChanged(-1);
    } else {
        m_variableList->setCurrentRow(0);
    }

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttons);

    connect(m_conditionEdit, &QTextEdit::textChanged, this, &EdgeDialog::updateValidation);
    connect(m_variableList, &QListWidget::currentRowChanged, this, &EdgeDialog::onVariableSelectionChanged);
    connect(m_variableConditionEdit, &QTextEdit::textChanged, this, [this]() {
        if (m_loadingVariableEditors) {
            return;
        }
        saveVariableEditorToCurrentRow();
        refreshVariableRowCaption(m_currentVariableRow);
        updateValidation();
    });
    connect(m_variableNewValueEdit, &QTextEdit::textChanged, this, [this]() {
        if (m_loadingVariableEditors) {
            return;
        }
        saveVariableEditorToCurrentRow();
        refreshVariableRowCaption(m_currentVariableRow);
        updateValidation();
    });

    updateValidation();
}

QString EdgeDialog::optionText() const {
    return m_optionEdit->toPlainText();
}

QString EdgeDialog::transitionText() const {
    return m_transitionEdit->toPlainText();
}

QString EdgeDialog::conditionText() const {
    return m_conditionEdit->toPlainText();
}

QVector<tge::domain::EdgeVariableSettingDef> EdgeDialog::variableSettings() const {
    QVector<tge::domain::EdgeVariableSettingDef> result;
    for (const auto& variableDef : m_globalVariables) {
        const int variableIndex = parseVariableIdentifierToIndex(variableDef.index);
        if (variableIndex <= 0) {
            continue;
        }

        auto it = m_variableSettingsByIndex.constFind(variableIndex);
        if (it == m_variableSettingsByIndex.constEnd()) {
            continue;
        }

        const QString condition = it.value().edgeVariableCondition.trimmed();
        const QString newValue = it.value().newValueFormula.trimmed();
        if (condition.isEmpty() && newValue.isEmpty()) {
            continue;
        }

        tge::domain::EdgeVariableSettingDef out;
        out.variableIndex = variableIndex;
        out.edgeVariableCondition = condition;
        out.newValueFormula = newValue;
        result.append(out);
    }
    return result;
}

int EdgeDialog::edgeColor() const {
    return m_selectedColor;
}

void EdgeDialog::updateValidation() {
    saveVariableEditorToCurrentRow();

    bool mainConditionOk = true;
    const QString condition = m_conditionEdit->toPlainText().trimmed();
    if (condition.isEmpty()) {
        m_conditionStatusLabel->setText(tr("Condition is empty: edge is always available"));
        m_conditionStatusLabel->setStyleSheet("color: #6a8f43;");
    } else {
        const auto parseResult = tge::formula::parse(condition.toStdString());
        if (parseResult.ast) {
            m_conditionStatusLabel->setText(tr("Formula parsed successfully"));
            m_conditionStatusLabel->setStyleSheet("color: #1d7d31;");
        } else {
            m_conditionStatusLabel->setText(tr("Parse error: %1").arg(QString::fromStdString(parseResult.error)));
            m_conditionStatusLabel->setStyleSheet("color: #b00020;");
            mainConditionOk = false;
        }
    }

    bool selectedConditionOk = true;
    bool selectedNewValueOk = true;
    if (m_currentVariableRow >= 0 && m_currentVariableRow < m_globalVariables.size()) {
        const QString selectedCondition = m_variableConditionEdit->toPlainText().trimmed();
        if (selectedCondition.isEmpty()) {
            m_variableConditionStatusLabel->setText(tr("Condition is empty: no extra restriction"));
            m_variableConditionStatusLabel->setStyleSheet("color: #6a8f43;");
        } else {
            const auto parseResult = tge::formula::parse(selectedCondition.toStdString());
            if (parseResult.ast) {
                m_variableConditionStatusLabel->setText(tr("Condition formula parsed successfully"));
                m_variableConditionStatusLabel->setStyleSheet("color: #1d7d31;");
            } else {
                m_variableConditionStatusLabel->setText(tr("Condition parse error: %1").arg(QString::fromStdString(parseResult.error)));
                m_variableConditionStatusLabel->setStyleSheet("color: #b00020;");
                selectedConditionOk = false;
            }
        }

        const QString selectedNewValue = m_variableNewValueEdit->toPlainText().trimmed();
        if (selectedNewValue.isEmpty()) {
            m_variableNewValueStatusLabel->setText(tr("Value formula is empty: parameter is not changed"));
            m_variableNewValueStatusLabel->setStyleSheet("color: #6a8f43;");
        } else {
            const auto parseResult = tge::formula::parse(selectedNewValue.toStdString());
            if (parseResult.ast) {
                m_variableNewValueStatusLabel->setText(tr("Value formula parsed successfully"));
                m_variableNewValueStatusLabel->setStyleSheet("color: #1d7d31;");
            } else {
                m_variableNewValueStatusLabel->setText(tr("Value parse error: %1").arg(QString::fromStdString(parseResult.error)));
                m_variableNewValueStatusLabel->setStyleSheet("color: #b00020;");
                selectedNewValueOk = false;
            }
        }
    } else {
        m_variableConditionStatusLabel->setText(tr("No global variables available"));
        m_variableConditionStatusLabel->setStyleSheet("color: #6a8f43;");
        m_variableNewValueStatusLabel->clear();
    }

    QString allSettingsError;
    const bool allSettingsOk = validateAllVariableSettings(&allSettingsError);
    if (allSettingsOk) {
        m_variableOverallStatusLabel->setText(tr("All variable settings are valid"));
        m_variableOverallStatusLabel->setStyleSheet("color: #1d7d31;");
    } else {
        m_variableOverallStatusLabel->setText(allSettingsError);
        m_variableOverallStatusLabel->setStyleSheet("color: #b00020;");
    }

    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(mainConditionOk && selectedConditionOk && selectedNewValueOk && allSettingsOk);
}

void EdgeDialog::onColorButtonClicked(int id) {
    if (id < tge::domain::LOCATION_COLOR_COUNT) {
        m_selectedColor = id;
    } else {
        m_selectedColor = tge::domain::LOCATION_COLOR_NONE;
    }
    updateColorSelection();
}

void EdgeDialog::updateColorSelection() {
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        if ((m_selectedColor == tge::domain::LOCATION_COLOR_NONE && i == tge::domain::LOCATION_COLOR_COUNT) ||
            (m_selectedColor == i)) {
            m_colorButtons[i]->setStyleSheet(m_colorButtons[i]->styleSheet() + "; border: 3px solid #333;");
        } else {
            if (i < tge::domain::LOCATION_COLOR_COUNT) {
                m_colorButtons[i]->setStyleSheet(QString("background-color: %1; border: 1px solid #aaa;")
                                                     .arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            } else {
                m_colorButtons[i]->setStyleSheet("background: none; border: 1px dashed #aaa;");
            }
        }
    }
}

void EdgeDialog::rebuildVariableList() {
    m_variableList->clear();
    for (int i = 0; i < m_globalVariables.size(); ++i) {
        const auto& variable = m_globalVariables[i];
        const QString shownName = variable.name.trimmed().isEmpty() ? tr("(no name)") : variable.name;
        m_variableList->addItem(QString("%1 | %2").arg(variable.index, shownName));
        refreshVariableRowCaption(i);
    }
}

void EdgeDialog::refreshVariableRowCaption(int row) {
    if (row < 0 || row >= m_globalVariables.size()) {
        return;
    }

    QListWidgetItem* item = m_variableList->item(row);
    if (!item) {
        return;
    }

    const auto& variable = m_globalVariables[row];
    const QString shownName = variable.name.trimmed().isEmpty() ? tr("(no name)") : variable.name;
    const int variableIndex = parseVariableIdentifierToIndex(variable.index);
    const auto it = m_variableSettingsByIndex.constFind(variableIndex);
    const bool hasSettings = (it != m_variableSettingsByIndex.constEnd())
        && !(it.value().edgeVariableCondition.trimmed().isEmpty() && it.value().newValueFormula.trimmed().isEmpty());

    item->setText(QString("%1 | %2%3")
                      .arg(variable.index, shownName, hasSettings ? tr("  [configured]") : QString()));
}

int EdgeDialog::variableIndexForRow(int row) const {
    if (row < 0 || row >= m_globalVariables.size()) {
        return -1;
    }
    return parseVariableIdentifierToIndex(m_globalVariables[row].index);
}

void EdgeDialog::saveVariableEditorToCurrentRow() {
    const int variableIndex = variableIndexForRow(m_currentVariableRow);
    if (variableIndex <= 0) {
        return;
    }

    const QString condition = m_variableConditionEdit->toPlainText().trimmed();
    const QString newValue = m_variableNewValueEdit->toPlainText().trimmed();
    if (condition.isEmpty() && newValue.isEmpty()) {
        m_variableSettingsByIndex.remove(variableIndex);
        return;
    }

    tge::domain::EdgeVariableSettingDef setting;
    setting.variableIndex = variableIndex;
    setting.edgeVariableCondition = condition;
    setting.newValueFormula = newValue;
    m_variableSettingsByIndex.insert(variableIndex, setting);
}

void EdgeDialog::loadVariableRowToEditor(int row) {
    m_loadingVariableEditors = true;

    const int variableIndex = variableIndexForRow(row);
    if (variableIndex <= 0) {
        m_variableConditionEdit->clear();
        m_variableNewValueEdit->clear();
        m_loadingVariableEditors = false;
        return;
    }

    auto it = m_variableSettingsByIndex.constFind(variableIndex);
    if (it == m_variableSettingsByIndex.constEnd()) {
        m_variableConditionEdit->clear();
        m_variableNewValueEdit->clear();
    } else {
        m_variableConditionEdit->setPlainText(it.value().edgeVariableCondition);
        m_variableNewValueEdit->setPlainText(it.value().newValueFormula);
    }

    m_loadingVariableEditors = false;
}

void EdgeDialog::updateVariableEditorsEnabledState() {
    const bool enabled = (m_currentVariableRow >= 0 && m_currentVariableRow < m_globalVariables.size());
    m_variableConditionEdit->setEnabled(enabled);
    m_variableNewValueEdit->setEnabled(enabled);
}

void EdgeDialog::onVariableSelectionChanged(int row) {
    if (!m_loadingVariableEditors) {
        saveVariableEditorToCurrentRow();
        refreshVariableRowCaption(m_currentVariableRow);
    }

    m_currentVariableRow = row;
    loadVariableRowToEditor(row);
    updateVariableEditorsEnabledState();
    updateValidation();
}

bool EdgeDialog::validateAllVariableSettings(QString* errorMessage) const {
    QMap<int, std::shared_ptr<tge::formula::ASTNode>> valueFormulaAsts;

    for (auto it = m_variableSettingsByIndex.constBegin(); it != m_variableSettingsByIndex.constEnd(); ++it) {
        const auto& setting = it.value();

        const QString condition = setting.edgeVariableCondition.trimmed();
        if (!condition.isEmpty()) {
            const auto parseResult = tge::formula::parse(condition.toStdString());
            if (!parseResult.ast) {
                if (errorMessage) {
                    *errorMessage = tr("%1: condition parse error: %2")
                                        .arg(QString("P%1").arg(setting.variableIndex), QString::fromStdString(parseResult.error));
                }
                return false;
            }
        }

        const QString newValue = setting.newValueFormula.trimmed();
        if (!newValue.isEmpty()) {
            const auto parseResult = tge::formula::parse(newValue.toStdString());
            if (!parseResult.ast) {
                if (errorMessage) {
                    *errorMessage = tr("%1: value parse error: %2")
                                        .arg(QString("P%1").arg(setting.variableIndex), QString::fromStdString(parseResult.error));
                }
                return false;
            }
            valueFormulaAsts.insert(setting.variableIndex, parseResult.ast);
        }
    }

    QMap<int, QSet<int>> dependencies;
    for (auto it = valueFormulaAsts.constBegin(); it != valueFormulaAsts.constEnd(); ++it) {
        QSet<int> refs;
        collectParameterReferences(it.value(), refs);

        QSet<int> relevantRefs;
        for (int ref : refs) {
            if (valueFormulaAsts.contains(ref)) {
                relevantRefs.insert(ref);
            }
        }
        dependencies.insert(it.key(), relevantRefs);
    }

    QMap<int, int> visitState; // 0 = not visited, 1 = in stack, 2 = done
    std::function<bool(int, int&)> dfs = [&](int node, int& cycleNode) -> bool {
        visitState[node] = 1;
        for (int dep : dependencies.value(node)) {
            if (visitState.value(dep, 0) == 1) {
                cycleNode = dep;
                return true;
            }
            if (visitState.value(dep, 0) == 0 && dfs(dep, cycleNode)) {
                return true;
            }
        }
        visitState[node] = 2;
        return false;
    };

    for (auto it = dependencies.constBegin(); it != dependencies.constEnd(); ++it) {
        const int node = it.key();
        if (visitState.value(node, 0) != 0) {
            continue;
        }

        int cycleNode = -1;
        if (dfs(node, cycleNode)) {
            if (errorMessage) {
                const int shownNode = (cycleNode > 0) ? cycleNode : node;
                *errorMessage = tr("Cyclic dependency detected in value formulas (for example at P%1)").arg(shownNode);
            }
            return false;
        }
    }

    return true;
}

int EdgeDialog::parseVariableIdentifierToIndex(const QString& variableIdentifier) {
    const QString trimmed = variableIdentifier.trimmed();
    bool ok = false;
    int index = -1;
    if (trimmed.startsWith('P', Qt::CaseInsensitive)) {
        index = trimmed.mid(1).toInt(&ok);
    } else {
        index = trimmed.toInt(&ok);
    }
    if (!ok || index <= 0) {
        return -1;
    }
    return index;
}

void EdgeDialog::collectParameterReferences(const std::shared_ptr<tge::formula::ASTNode>& node, QSet<int>& outRefs) {
    if (!node) {
        return;
    }

    if (node->type == tge::formula::ASTNode::Type::Parameter) {
        const int refIndex = parseVariableIdentifierToIndex(QString::fromStdString(node->value));
        if (refIndex > 0) {
            outRefs.insert(refIndex);
        }
    }

    collectParameterReferences(node->left, outRefs);
    collectParameterReferences(node->right, outRefs);
}
