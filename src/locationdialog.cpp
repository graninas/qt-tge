#include "locationdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "tge/domain.h"

LocationDialog::LocationDialog(const tge::domain::LocationDef& loc, const QString& typeStr, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Location Info"));
    QVBoxLayout* layout = new QVBoxLayout(this);

    // ID and coords (read-only)
    QString idType = QString("%1 (%2): (%3,%4)").arg(loc.id).arg(typeStr).arg(loc.coordX).arg(loc.coordY);
    QLabel* idLabel = new QLabel(idType, this);
    layout->addWidget(idLabel);

    // Label
    layout->addWidget(new QLabel(tr("Label:"), this));
    m_labelEdit = new QLineEdit(loc.label, this);
    layout->addWidget(m_labelEdit);

    // Description
    layout->addWidget(new QLabel(tr("Description:"), this));
    m_descEdit = new QTextEdit(this);
    QString desc;
    if (!loc.descriptionPack.descriptions.isEmpty())
        desc = loc.descriptionPack.descriptions[0];
    m_descEdit->setPlainText(desc);
    layout->addWidget(m_descEdit);

    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString LocationDialog::label() const {
    return m_labelEdit->text();
}

QString LocationDialog::description() const {
    return m_descEdit->toPlainText();
}
