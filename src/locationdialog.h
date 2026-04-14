#pragma once
#include <QDialog>
#include <QString>
#include <QListWidget>
#include <QMap>
#include "tge/domain.h"
#include "tge/editor/runtime/manager.h"
#include <QButtonGroup>
#include <QVBoxLayout>

namespace tge {
namespace domain {
struct LocationDef;
}
namespace editor {
namespace runtime {
class Manager;
}
}
}

class QLineEdit;
class QTextEdit;
class QTabWidget;
class QPushButton;

class LocationDialog : public QDialog {
    Q_OBJECT
public:
    LocationDialog(tge::domain::LocationDef* loc, tge::editor::runtime::Manager* manager, QWidget* parent = nullptr);
    QString label() const;
    QString description() const;
    QList<QString> descriptions() const;
private slots:
    void onEdgeDeleteRequested(int edgeId);
    void onEdgeItemClicked(QListWidgetItem* item);
    void onColorButtonClicked(int id);
private:
    QLineEdit* m_labelEdit;
    QTabWidget* m_descTabs;
    QPushButton* m_addDescBtn;
    QPushButton* m_removeDescBtn;
    QListWidget* m_edgeListWidget;
    tge::domain::LocationDef* m_location;
    tge::editor::runtime::Manager* m_manager;
    QButtonGroup* m_colorButtonGroup;
    QVector<QPushButton*> m_colorButtons;
    void addDescriptionTab(const QString& text = QString());
    void removeLastDescriptionTab();
    void populateEdgeList();
    void setupColorPaletteUI(QVBoxLayout* layout);
    void updateColorSelection();
};
