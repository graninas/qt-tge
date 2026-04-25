#ifndef INFODISPLAYITEMSDIALOG_H
#define INFODISPLAYITEMSDIALOG_H

#include <QDialog>
#include <QVector>

#include "tge/domain.h"

class QListWidget;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QDialogButtonBox;

class InfoDisplayItemsDialog : public QDialog {
    Q_OBJECT

public:
    explicit InfoDisplayItemsDialog(QVector<tge::domain::InfoDisplayItemDef>& items, QWidget* parent = nullptr);

private:
    void rebuildList();
    void updateEditorEnabledState();
    void saveEditorToCurrentRow();
    void loadRowToEditor(int row);
    void refreshRowCaption(int row);
    void validateAndShowStatus();

    bool validateAll(QString* errorMessage = nullptr) const;

    void onAddItem();
    void onRemoveItem();
    void onSelectionChanged(int newRow);
    void onEditorChanged();
    void onAccepted();

private:
    QVector<tge::domain::InfoDisplayItemDef>& m_itemsRef;
    QVector<tge::domain::InfoDisplayItemDef> m_workingItems;

    int m_currentRow = -1;
    bool m_loadingEditor = false;

    QListWidget* m_listWidget = nullptr;
    QPushButton* m_addButton = nullptr;
    QPushButton* m_removeButton = nullptr;

    QLineEdit* m_labelEdit = nullptr;
    QTextEdit* m_formulaEdit = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QLabel* m_formulaStatusLabel = nullptr;

    QLabel* m_statusLabel = nullptr;
    QDialogButtonBox* m_buttonBox = nullptr;
};

#endif // INFODISPLAYITEMSDIALOG_H
