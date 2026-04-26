#pragma once
#include <QDialog>
#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>
#include <memory>

#include "tge/domain.h"

namespace tge {
namespace domain {
struct EdgeDef;
struct LocationDef;
}
namespace formula {
struct ASTNode;
}
}

class QTextEdit;
class QLabel;
class QDialogButtonBox;
class QButtonGroup;
class QPushButton;
class QListWidget;
class QCheckBox;
class QSpinBox;
class QComboBox;

class EdgeDialog : public QDialog {
    Q_OBJECT
public:
    EdgeDialog(const tge::domain::EdgeDef& edge,
               const tge::domain::LocationDef& fromLoc,
               const tge::domain::LocationDef& toLoc,
               const QVector<tge::domain::VariableDef>& globalVariables,
               const QVector<tge::domain::InfoDisplayItemDef>& infoDisplayItems,
               QWidget* parent = nullptr);
    QString optionText() const;
    QString transitionText() const;
    QString conditionText() const;
    QVector<tge::domain::EdgeVariableSettingDef> variableSettings() const;
    QVector<tge::domain::EdgeInfoDisplayItemSettingDef> infoDisplayItemSettings() const;
    int edgeColor() const;
    int edgePriority() const;
private:
    void updateValidation();
    void onColorButtonClicked(int id);
    void updateColorSelection();

    void rebuildVariableList();
    void refreshVariableRowCaption(int row);
    int variableIndexForRow(int row) const;
    void saveVariableEditorToCurrentRow();
    void loadVariableRowToEditor(int row);
    void updateVariableEditorsEnabledState();
    void onVariableSelectionChanged(int row);

    void rebuildInfoDisplayItemList();
    void refreshInfoDisplayItemRowCaption(int row);
    int infoDisplayItemIdForRow(int row) const;
    void saveInfoDisplayItemEditorToCurrentRow();
    void loadInfoDisplayItemRowToEditor(int row);
    void updateInfoDisplayItemEditorsEnabledState();
    void onInfoDisplayItemSelectionChanged(int row);

    bool validateAllVariableSettings(QString* errorMessage = nullptr) const;
    bool validateAllInfoDisplayItemSettings(QString* errorMessage = nullptr) const;
    static int parseVariableIdentifierToIndex(const QString& variableIdentifier);
    static void collectParameterReferences(const std::shared_ptr<tge::formula::ASTNode>& node, QSet<int>& outRefs);

    QLabel* m_idLabel;
    QLabel* m_fromLabel;
    QLabel* m_toLabel;
    QTextEdit* m_optionEdit;
    QTextEdit* m_transitionEdit;
    QTextEdit* m_conditionEdit;
    QLabel* m_conditionStatusLabel;
    QDialogButtonBox* m_buttons;
    QButtonGroup* m_colorButtonGroup;
    QVector<QPushButton*> m_colorButtons;

    const QVector<tge::domain::VariableDef>& m_globalVariables;
    const QVector<tge::domain::InfoDisplayItemDef>& m_infoDisplayItems;
    QMap<int, tge::domain::EdgeVariableSettingDef> m_variableSettingsByIndex;
    QMap<int, tge::domain::EdgeInfoDisplayItemSettingDef> m_infoDisplaySettingsById;
    int m_currentVariableRow = -1;
    int m_currentInfoDisplayRow = -1;
    bool m_loadingVariableEditors = false;
    bool m_loadingInfoDisplayEditors = false;
    QListWidget* m_variableList;
    QTextEdit* m_variableConditionEdit;
    QTextEdit* m_variableNewValueEdit;
    QLabel* m_variableConditionStatusLabel;
    QLabel* m_variableNewValueStatusLabel;
    QLabel* m_variableOverallStatusLabel;

    QListWidget* m_infoDisplayItemList;
    QCheckBox* m_changePriorityCheck;
    QSpinBox* m_newPrioritySpin;
    QCheckBox* m_changeVisibilityCheck;
    QComboBox* m_newVisibilityCombo;
    QCheckBox* m_changeShowValueCheck;
    QComboBox* m_newShowValueCombo;
    QLabel* m_infoDisplayValueFormulaLabel;
    QLabel* m_infoDisplayOverallStatusLabel;

    QSpinBox* m_prioritySpin;
    int m_selectedColor;
};
