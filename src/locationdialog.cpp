#include "locationdialog.h"
#include "edgedialog.h"
#include "edgelistitemwidget.h"
#include "tge/editor/runtime/manager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>

using tge::editor::runtime::Manager;

LocationDialog::LocationDialog(tge::domain::LocationDef* loc, Manager* manager, QWidget* parent)
    : QDialog(parent), m_location(loc), m_manager(manager)
{
    setWindowTitle(tr("Location Info"));
    QVBoxLayout* layout = new QVBoxLayout(this);

    // ID and coords (read-only)
    QString idType = QString("%1 (%2): (%3,%4)").arg(loc->id).arg(static_cast<int>(loc->type)).arg(loc->coordX).arg(loc->coordY);
    QLabel* idLabel = new QLabel(idType, this);
    layout->addWidget(idLabel);

    // Label
    layout->addWidget(new QLabel(tr("Label:"), this));
    m_labelEdit = new QLineEdit(loc->label, this);
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
    int idx = 1;
    if (!loc->descriptionPack.descriptions.isEmpty()) {
        for (const QString& desc : loc->descriptionPack.descriptions) {
            addDescriptionTab(desc);
            ++idx;
        }
    } else {
        addDescriptionTab();
    }
    connect(m_addDescBtn, &QPushButton::clicked, this, [this]() { addDescriptionTab(); });
    connect(m_removeDescBtn, &QPushButton::clicked, this, [this]() { removeLastDescriptionTab(); });

    // --- Edges area ---
    layout->addWidget(new QLabel(tr("Edges (incoming/outgoing):"), this));
    m_edgeListWidget = new QListWidget(this);
    layout->addWidget(m_edgeListWidget);
    populateEdgeList();
    connect(m_edgeListWidget, &QListWidget::itemClicked, this, &LocationDialog::onEdgeItemClicked);

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

void LocationDialog::populateEdgeList() {
    m_edgeListWidget->clear();
    // Outgoing and incoming edges
    auto& game = m_manager->game();
    for (auto it = game.edges.begin(); it != game.edges.end(); ++it) {
        const auto& edge = it.value();
        if (edge.fromLocation == m_location->id || edge.toLocation == m_location->id) {
            EdgeListItemWidget* widget = new EdgeListItemWidget(edge, m_location->id, m_edgeListWidget);
            QListWidgetItem* item = new QListWidgetItem(m_edgeListWidget);
            item->setSizeHint(widget->sizeHint());
            m_edgeListWidget->addItem(item);
            m_edgeListWidget->setItemWidget(item, widget);
            connect(widget, &EdgeListItemWidget::deleteRequested, this, &LocationDialog::onEdgeDeleteRequested);
        }
    }
}

void LocationDialog::onEdgeDeleteRequested(int edgeId) {
    if (QMessageBox::question(this, tr("Delete Edge"), tr("Delete edge %1?").arg(edgeId)) == QMessageBox::Yes) {
        m_manager->deleteEdge(edgeId);
        populateEdgeList();
    }
}

void LocationDialog::onEdgeItemClicked(QListWidgetItem* item) {
    // Open edge dialog for editing
    auto& game = m_manager->game();
    EdgeListItemWidget* widget = qobject_cast<EdgeListItemWidget*>(m_edgeListWidget->itemWidget(item));
    if (!widget) return;
    int edgeId = widget->edgeId();
    if (!game.edges.contains(edgeId)) return;
    auto& edge = game.edges[edgeId];
    auto& fromLoc = game.locations[edge.fromLocation];
    auto& toLoc = game.locations[edge.toLocation];
    EdgeDialog dlg(edge, fromLoc, toLoc, this);
    if (dlg.exec() == QDialog::Accepted) {
        edge.optionText = dlg.optionText();
        edge.transitionText = dlg.transitionText();
        populateEdgeList();
    }
}
