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

class LocationDialog : public QDialog {
    Q_OBJECT
public:
    LocationDialog(const tge::domain::LocationDef& loc, const QString& typeStr, QWidget* parent = nullptr);
    QString label() const;
    QString description() const;
private:
    QLineEdit* m_labelEdit;
    QTextEdit* m_descEdit;
};
