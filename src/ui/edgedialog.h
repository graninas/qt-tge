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

class EdgeDialog : public QDialog {
    Q_OBJECT
public:
    EdgeDialog(const tge::domain::EdgeDef& edge,
               const tge::domain::LocationDef& fromLoc,
               const tge::domain::LocationDef& toLoc,
               const QVector<tge::domain::VariableDef>& globalVariables,
               QWidget* parent = nullptr);
    QString optionText() const;
    QString transitionText() const;
    QString conditionText() const;
    QVector<tge::domain::EdgeVariableSettingDef> variableSettings() const;
    int edgeColor() const;
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

    bool validateAllVariableSettings(QString* errorMessage = nullptr) const;
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
    QMap<int, tge::domain::EdgeVariableSettingDef> m_variableSettingsByIndex;
    int m_currentVariableRow = -1;
    bool m_loadingVariableEditors = false;
    QListWidget* m_variableList;
    QTextEdit* m_variableConditionEdit;
    QTextEdit* m_variableNewValueEdit;
    QLabel* m_variableConditionStatusLabel;
    QLabel* m_variableNewValueStatusLabel;
    QLabel* m_variableOverallStatusLabel;

    int m_selectedColor;
};
