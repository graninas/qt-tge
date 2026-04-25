#include "edgedialog.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
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
                       const QVector<tge::domain::InfoDisplayItemDef>& infoDisplayItems,
                       QWidget* parent)
    : QDialog(parent)
    , m_colorButtonGroup(nullptr)
    , m_globalVariables(globalVariables)
    , m_infoDisplayItems(infoDisplayItems)
    , m_variableList(nullptr)
    , m_variableConditionEdit(nullptr)
    , m_variableNewValueEdit(nullptr)
    , m_variableConditionStatusLabel(nullptr)
    , m_variableNewValueStatusLabel(nullptr)
    , m_variableOverallStatusLabel(nullptr)
    , m_infoDisplayItemList(nullptr)
    , m_changePriorityCheck(nullptr)
    , m_newPrioritySpin(nullptr)
    , m_changeVisibilityCheck(nullptr)
    , m_newVisibilityCombo(nullptr)
    , m_changeShowValueCheck(nullptr)
    , m_newShowValueCombo(nullptr)
    , m_infoDisplayNewValueEdit(nullptr)
    , m_infoDisplayNewValueStatusLabel(nullptr)
    , m_infoDisplayOverallStatusLabel(nullptr)
    , m_selectedColor(edge.color)
{
    for (const auto& setting : edge.variableSettings) {
        m_variableSettingsByIndex.insert(setting.variableIndex, setting);
    }
    for (const auto& setting : edge.infoDisplayItemSettings) {
        m_infoDisplaySettingsById.insert(setting.itemIndex, setting);
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
        m_currentVariableRow = -1;
        loadVariableRowToEditor(-1);
        updateVariableEditorsEnabledState();
    } else {
        m_variableList->setCurrentRow(0);
    }

    QGroupBox* infoDisplayGroup = new QGroupBox(tr("Info Display Item Settings"), this);
    QHBoxLayout* infoDisplayRoot = new QHBoxLayout(infoDisplayGroup);

    m_infoDisplayItemList = new QListWidget(infoDisplayGroup);
    infoDisplayRoot->addWidget(m_infoDisplayItemList, 2);

    QVBoxLayout* infoDisplayEditorLayout = new QVBoxLayout();

    m_changePriorityCheck = new QCheckBox(tr("Change priority"), infoDisplayGroup);
    infoDisplayEditorLayout->addWidget(m_changePriorityCheck);

    m_newPrioritySpin = new QSpinBox(infoDisplayGroup);
    m_newPrioritySpin->setMinimum(-1000000000);
    m_newPrioritySpin->setMaximum(1000000000);
    infoDisplayEditorLayout->addWidget(m_newPrioritySpin);

    m_changeVisibilityCheck = new QCheckBox(tr("Change visibility"), infoDisplayGroup);
    infoDisplayEditorLayout->addWidget(m_changeVisibilityCheck);

    m_newVisibilityCombo = new QComboBox(infoDisplayGroup);
    m_newVisibilityCombo->addItem(tr("Hidden"), 0);
    m_newVisibilityCombo->addItem(tr("Shown"), 1);
    infoDisplayEditorLayout->addWidget(m_newVisibilityCombo);

    m_changeShowValueCheck = new QCheckBox(tr("Change show value"), infoDisplayGroup);
    infoDisplayEditorLayout->addWidget(m_changeShowValueCheck);

    m_newShowValueCombo = new QComboBox(infoDisplayGroup);
    m_newShowValueCombo->addItem(tr("Hide formula value"), 0);
    m_newShowValueCombo->addItem(tr("Show formula value"), 1);
    infoDisplayEditorLayout->addWidget(m_newShowValueCombo);

    infoDisplayEditorLayout->addWidget(new QLabel(tr("New Value Formula:"), infoDisplayGroup));
    m_infoDisplayNewValueEdit = new QTextEdit(infoDisplayGroup);
    m_infoDisplayNewValueEdit->setMaximumHeight(55);
    infoDisplayEditorLayout->addWidget(m_infoDisplayNewValueEdit);

    m_infoDisplayNewValueStatusLabel = new QLabel(infoDisplayGroup);
    infoDisplayEditorLayout->addWidget(m_infoDisplayNewValueStatusLabel);

    m_infoDisplayOverallStatusLabel = new QLabel(infoDisplayGroup);
    infoDisplayEditorLayout->addWidget(m_infoDisplayOverallStatusLabel);
    infoDisplayEditorLayout->addStretch(1);

    infoDisplayRoot->addLayout(infoDisplayEditorLayout, 3);
    layout->addWidget(infoDisplayGroup);

    rebuildInfoDisplayItemList();
    if (m_infoDisplayItems.isEmpty()) {
        m_currentInfoDisplayRow = -1;
        loadInfoDisplayItemRowToEditor(-1);
        updateInfoDisplayItemEditorsEnabledState();
    } else {
        m_infoDisplayItemList->setCurrentRow(0);
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
    connect(m_infoDisplayItemList, &QListWidget::currentRowChanged, this, &EdgeDialog::onInfoDisplayItemSelectionChanged);
    connect(m_changePriorityCheck, &QCheckBox::toggled, this, [this](bool) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        updateInfoDisplayItemEditorsEnabledState();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_newPrioritySpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_changeVisibilityCheck, &QCheckBox::toggled, this, [this](bool) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        updateInfoDisplayItemEditorsEnabledState();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_newVisibilityCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_changeShowValueCheck, &QCheckBox::toggled, this, [this](bool) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        updateInfoDisplayItemEditorsEnabledState();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_newShowValueCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });
    connect(m_infoDisplayNewValueEdit, &QTextEdit::textChanged, this, [this]() {
        if (m_loadingInfoDisplayEditors) {
            return;
        }
        saveInfoDisplayItemEditorToCurrentRow();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
        updateValidation();
    });

    // Keep internal row trackers in sync with preselected list rows.
    if (!m_globalVariables.isEmpty()) {
        m_currentVariableRow = m_variableList->currentRow();
        loadVariableRowToEditor(m_currentVariableRow);
        updateVariableEditorsEnabledState();
    }
    if (!m_infoDisplayItems.isEmpty()) {
        m_currentInfoDisplayRow = m_infoDisplayItemList->currentRow();
        loadInfoDisplayItemRowToEditor(m_currentInfoDisplayRow);
        updateInfoDisplayItemEditorsEnabledState();
    }

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

QVector<tge::domain::EdgeInfoDisplayItemSettingDef> EdgeDialog::infoDisplayItemSettings() const {
    QVector<tge::domain::EdgeInfoDisplayItemSettingDef> result;
    for (const auto& itemDef : m_infoDisplayItems) {
        auto it = m_infoDisplaySettingsById.constFind(itemDef.id);
        if (it == m_infoDisplaySettingsById.constEnd()) {
            continue;
        }

        const auto& setting = it.value();
        const QString newValue = setting.newValueFormula.trimmed();
        if (!setting.changePriority && !setting.changeVisibility && !setting.changeShowValue && newValue.isEmpty()) {
            continue;
        }

        tge::domain::EdgeInfoDisplayItemSettingDef out = setting;
        out.itemIndex = itemDef.id;
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
    saveInfoDisplayItemEditorToCurrentRow();

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

    bool selectedInfoDisplayValueOk = true;
    if (m_currentInfoDisplayRow >= 0 && m_currentInfoDisplayRow < m_infoDisplayItems.size()) {
        const QString selectedNewValue = m_infoDisplayNewValueEdit->toPlainText().trimmed();
        if (selectedNewValue.isEmpty()) {
            m_infoDisplayNewValueStatusLabel->setText(tr("Value formula is empty: item value is not changed"));
            m_infoDisplayNewValueStatusLabel->setStyleSheet("color: #6a8f43;");
        } else {
            const auto parseResult = tge::formula::parse(selectedNewValue.toStdString());
            if (parseResult.ast) {
                m_infoDisplayNewValueStatusLabel->setText(tr("Value formula parsed successfully"));
                m_infoDisplayNewValueStatusLabel->setStyleSheet("color: #1d7d31;");
            } else {
                m_infoDisplayNewValueStatusLabel->setText(tr("Value parse error: %1").arg(QString::fromStdString(parseResult.error)));
                m_infoDisplayNewValueStatusLabel->setStyleSheet("color: #b00020;");
                selectedInfoDisplayValueOk = false;
            }
        }
    } else {
        m_infoDisplayNewValueStatusLabel->setText(tr("No info display items available"));
        m_infoDisplayNewValueStatusLabel->setStyleSheet("color: #6a8f43;");
    }

    QString allInfoDisplaySettingsError;
    const bool allInfoDisplaySettingsOk = validateAllInfoDisplayItemSettings(&allInfoDisplaySettingsError);
    if (allInfoDisplaySettingsOk) {
        m_infoDisplayOverallStatusLabel->setText(tr("All info display item settings are valid"));
        m_infoDisplayOverallStatusLabel->setStyleSheet("color: #1d7d31;");
    } else {
        m_infoDisplayOverallStatusLabel->setText(allInfoDisplaySettingsError);
        m_infoDisplayOverallStatusLabel->setStyleSheet("color: #b00020;");
    }

    if (m_buttons) {
        m_buttons->button(QDialogButtonBox::Ok)->setEnabled(
            mainConditionOk
            && selectedConditionOk
            && selectedNewValueOk
            && allSettingsOk
            && selectedInfoDisplayValueOk
            && allInfoDisplaySettingsOk);
    }
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

void EdgeDialog::rebuildInfoDisplayItemList() {
    m_infoDisplayItemList->clear();
    for (int i = 0; i < m_infoDisplayItems.size(); ++i) {
        const auto& infoItem = m_infoDisplayItems[i];
        const QString shownLabel = infoItem.label.trimmed().isEmpty() ? tr("(no label)") : infoItem.label;
        m_infoDisplayItemList->addItem(QString("#%1 | %2").arg(infoItem.id).arg(shownLabel));
        refreshInfoDisplayItemRowCaption(i);
    }
}

void EdgeDialog::refreshInfoDisplayItemRowCaption(int row) {
    if (row < 0 || row >= m_infoDisplayItems.size()) {
        return;
    }

    QListWidgetItem* item = m_infoDisplayItemList->item(row);
    if (!item) {
        return;
    }

    const auto& infoItem = m_infoDisplayItems[row];
    const QString shownLabel = infoItem.label.trimmed().isEmpty() ? tr("(no label)") : infoItem.label;
    const auto it = m_infoDisplaySettingsById.constFind(infoItem.id);
    const bool hasSettings = (it != m_infoDisplaySettingsById.constEnd())
        && (it.value().changePriority
            || it.value().changeVisibility
            || it.value().changeShowValue
            || !it.value().newValueFormula.trimmed().isEmpty());

    item->setText(QString("#%1 | %2%3")
                      .arg(infoItem.id)
                      .arg(shownLabel)
                      .arg(hasSettings ? tr("  [configured]") : QString()));
}

int EdgeDialog::infoDisplayItemIdForRow(int row) const {
    if (row < 0 || row >= m_infoDisplayItems.size()) {
        return -1;
    }
    return m_infoDisplayItems[row].id;
}

void EdgeDialog::saveInfoDisplayItemEditorToCurrentRow() {
    const int itemId = infoDisplayItemIdForRow(m_currentInfoDisplayRow);
    if (itemId < 0) {
        return;
    }

    tge::domain::EdgeInfoDisplayItemSettingDef setting;
    setting.itemIndex = itemId;
    setting.changePriority = m_changePriorityCheck->isChecked();
    setting.newPriority = m_newPrioritySpin->value();
    setting.changeVisibility = m_changeVisibilityCheck->isChecked();
    setting.newVisibility = (m_newVisibilityCombo->currentData().toInt() != 0);
    setting.changeShowValue = m_changeShowValueCheck->isChecked();
    setting.newShowValue = (m_newShowValueCombo->currentData().toInt() != 0);
    setting.newValueFormula = m_infoDisplayNewValueEdit->toPlainText().trimmed();

    if (!setting.changePriority
        && !setting.changeVisibility
        && !setting.changeShowValue
        && setting.newValueFormula.isEmpty()) {
        m_infoDisplaySettingsById.remove(itemId);
        return;
    }

    m_infoDisplaySettingsById.insert(itemId, setting);
}

void EdgeDialog::loadInfoDisplayItemRowToEditor(int row) {
    m_loadingInfoDisplayEditors = true;

    if (row < 0 || row >= m_infoDisplayItems.size()) {
        m_changePriorityCheck->setChecked(false);
        m_newPrioritySpin->setValue(0);
        m_changeVisibilityCheck->setChecked(false);
        m_newVisibilityCombo->setCurrentIndex(0);
        m_changeShowValueCheck->setChecked(false);
        m_newShowValueCombo->setCurrentIndex(0);
        m_infoDisplayNewValueEdit->clear();
        m_loadingInfoDisplayEditors = false;
        return;
    }

    const auto& infoItem = m_infoDisplayItems[row];
    auto it = m_infoDisplaySettingsById.constFind(infoItem.id);
    if (it == m_infoDisplaySettingsById.constEnd()) {
        m_changePriorityCheck->setChecked(false);
        m_newPrioritySpin->setValue(infoItem.priority);
        m_changeVisibilityCheck->setChecked(false);
        m_newVisibilityCombo->setCurrentIndex(infoItem.isVisible ? 1 : 0);
        m_changeShowValueCheck->setChecked(false);
        m_newShowValueCombo->setCurrentIndex(infoItem.showFormulaValue ? 1 : 0);
        m_infoDisplayNewValueEdit->clear();
    } else {
        const auto& setting = it.value();
        m_changePriorityCheck->setChecked(setting.changePriority);
        m_newPrioritySpin->setValue(setting.newPriority);
        m_changeVisibilityCheck->setChecked(setting.changeVisibility);
        m_newVisibilityCombo->setCurrentIndex(setting.newVisibility ? 1 : 0);
        m_changeShowValueCheck->setChecked(setting.changeShowValue);
        m_newShowValueCombo->setCurrentIndex(setting.newShowValue ? 1 : 0);
        m_infoDisplayNewValueEdit->setPlainText(setting.newValueFormula);
    }

    m_loadingInfoDisplayEditors = false;
}

void EdgeDialog::updateInfoDisplayItemEditorsEnabledState() {
    const bool selected = (m_currentInfoDisplayRow >= 0 && m_currentInfoDisplayRow < m_infoDisplayItems.size());
    m_changePriorityCheck->setEnabled(selected);
    m_changeVisibilityCheck->setEnabled(selected);
    m_changeShowValueCheck->setEnabled(selected);
    m_newPrioritySpin->setEnabled(selected && m_changePriorityCheck->isChecked());
    m_newVisibilityCombo->setEnabled(selected && m_changeVisibilityCheck->isChecked());
    m_newShowValueCombo->setEnabled(selected && m_changeShowValueCheck->isChecked());
    m_infoDisplayNewValueEdit->setEnabled(selected);
}

void EdgeDialog::onInfoDisplayItemSelectionChanged(int row) {
    if (!m_loadingInfoDisplayEditors) {
        saveInfoDisplayItemEditorToCurrentRow();
        refreshInfoDisplayItemRowCaption(m_currentInfoDisplayRow);
    }

    m_currentInfoDisplayRow = row;
    loadInfoDisplayItemRowToEditor(row);
    updateInfoDisplayItemEditorsEnabledState();
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

bool EdgeDialog::validateAllInfoDisplayItemSettings(QString* errorMessage) const {
    QSet<int> knownIds;
    for (const auto& item : m_infoDisplayItems) {
        knownIds.insert(item.id);
    }

    for (auto it = m_infoDisplaySettingsById.constBegin(); it != m_infoDisplaySettingsById.constEnd(); ++it) {
        const auto& setting = it.value();
        if (!knownIds.contains(setting.itemIndex)) {
            if (errorMessage) {
                *errorMessage = tr("Unknown info display item id: %1").arg(setting.itemIndex);
            }
            return false;
        }

        const QString newValue = setting.newValueFormula.trimmed();
        if (!newValue.isEmpty()) {
            const auto parseResult = tge::formula::parse(newValue.toStdString());
            if (!parseResult.ast) {
                if (errorMessage) {
                    *errorMessage = tr("Info item #%1: value parse error: %2")
                                        .arg(setting.itemIndex)
                                        .arg(QString::fromStdString(parseResult.error));
                }
                return false;
            }
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
