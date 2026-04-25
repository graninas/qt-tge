#pragma once
#include <QDialog>
#include <QString>
#include <QVector>

namespace tge {
namespace domain {
struct EdgeDef;
struct LocationDef;
}
}

class QTextEdit;
class QLabel;
class QDialogButtonBox;
class QButtonGroup;
class QPushButton;

class EdgeDialog : public QDialog {
    Q_OBJECT
public:
    EdgeDialog(const tge::domain::EdgeDef& edge,
               const tge::domain::LocationDef& fromLoc,
               const tge::domain::LocationDef& toLoc,
               QWidget* parent = nullptr);
    QString optionText() const;
    QString transitionText() const;
    QString conditionText() const;
    int edgeColor() const;
private:
    void updateConditionValidation();
    void onColorButtonClicked(int id);
    void updateColorSelection();

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
    int m_selectedColor;
};
