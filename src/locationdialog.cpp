#include "locationdialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QPushButton>
#include <QHBoxLayout>
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

    // Description tabs
    layout->addWidget(new QLabel(tr("Descriptions:"), this));
    QHBoxLayout* descBtnLayout = new QHBoxLayout;
    m_addDescBtn = new QPushButton("+", this);
    m_removeDescBtn = new QPushButton("-", this);
    descBtnLayout->addWidget(m_addDescBtn);
    descBtnLayout->addWidget(m_removeDescBtn);
    layout->addLayout(descBtnLayout);
    m_descTabs = new QTabWidget(this);
    layout->addWidget(m_descTabs);
    // Fill tabs from descriptionPack
    int idx = 1;
    if (!loc.descriptionPack.descriptions.isEmpty()) {
        for (const QString& desc : loc.descriptionPack.descriptions) {
            addDescriptionTab(desc);
            ++idx;
        }
    } else {
        addDescriptionTab();
    }
    connect(m_addDescBtn, &QPushButton::clicked, this, [this]() { addDescriptionTab(); });
    connect(m_removeDescBtn, &QPushButton::clicked, this, [this]() { removeLastDescriptionTab(); });

    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString LocationDialog::label() const {
    return m_labelEdit->text();
}

QList<QString> LocationDialog::descriptions() const {
    QList<QString> result;
    for (int i = 0; i < m_descTabs->count(); ++i) {
        QTextEdit* edit = qobject_cast<QTextEdit*>(m_descTabs->widget(i));
        result.append(edit ? edit->toPlainText() : QString());
    }
    return result;
}

void LocationDialog::addDescriptionTab(const QString& text) {
    QTextEdit* edit = new QTextEdit(this);
    edit->setPlainText(text);
    int idx = m_descTabs->count() + 1;
    m_descTabs->addTab(edit, tr("Description %1").arg(idx));
    m_descTabs->setCurrentWidget(edit);
}

void LocationDialog::removeLastDescriptionTab() {
    int count = m_descTabs->count();
    if (count > 1) {
        QWidget* last = m_descTabs->widget(count - 1);
        m_descTabs->removeTab(count - 1);
        delete last;
    }
}
