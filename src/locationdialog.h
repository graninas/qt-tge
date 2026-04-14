#pragma once
#include <QDialog>
#include <QString>

namespace tge {
namespace domain {
struct LocationDef;
}
}

class QLineEdit;
class QTextEdit;
class QTabWidget;
class QPushButton;

class LocationDialog : public QDialog {
    Q_OBJECT
public:
    LocationDialog(const tge::domain::LocationDef& loc, const QString& typeStr, QWidget* parent = nullptr);
    QString label() const;
    QString description() const;
    QList<QString> descriptions() const;
private:
    QLineEdit* m_labelEdit;
    QTabWidget* m_descTabs;
    QPushButton* m_addDescBtn;
    QPushButton* m_removeDescBtn;
    void addDescriptionTab(const QString& text = QString());
    void removeLastDescriptionTab();
};
