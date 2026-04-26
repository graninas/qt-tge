#ifndef GLOBALVARIABLESDIALOG_H
#define GLOBALVARIABLESDIALOG_H

#include <QDialog>
#include <QVector>

#include "tge/domain.h"
#include "tge/editor/types.h"

class QListWidget;
class QSpinBox;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QDialogButtonBox;

class GlobalVariablesDialog : public QDialog {
    Q_OBJECT

public:
    explicit GlobalVariablesDialog(QVector<tge::domain::VariableDef>& variables,
                                   const tge::editor::CapabilityMatrix& capabilities,
                                   QWidget* parent = nullptr);

private:
    void rebuildList();
    void updateEditorEnabledState();
    void saveEditorToCurrentRow();
    void loadRowToEditor(int row);
    void refreshRowCaption(int row);
    void validateAndShowStatus();

    bool validateAll(QString* errorMessage = nullptr) const;
    int firstFreeIndex() const;
    static int parseIndexToInt(const QString& indexText);
    static QString indexToText(int indexValue);

    void onAddVariable();
    void onRemoveVariable();
    void onSelectionChanged(int newRow);
    void onEditorChanged();
    void onAccepted();

private:
    QVector<tge::domain::VariableDef>& m_variablesRef;
    QVector<tge::domain::VariableDef> m_workingVariables;
    tge::editor::CapabilityMatrix m_capabilities;

    int m_currentRow = -1;
    bool m_loadingEditor = false;

    QListWidget* m_listWidget = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_removeButton = nullptr;

    QSpinBox* m_indexSpin = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QTextEdit* m_descriptionEdit = nullptr;
    QComboBox* m_typeCombo = nullptr;
    QLineEdit* m_defaultValueEdit = nullptr;

    QLabel* m_statusLabel = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};

#endif // GLOBALVARIABLESDIALOG_H
