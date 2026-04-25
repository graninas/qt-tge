#include "locationdialog.h"
#include "edgedialog.h"
#include "edgelistitemwidget.h"
#include "tge/editor/runtime/manager.h"
#include "graphwidget_helpers.h"
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
#include <QGridLayout>
#include <QButtonGroup>

using tge::editor::runtime::Manager;

LocationDialog::LocationDialog(tge::domain::LocationDef* loc, Manager* manager, QWidget* parent)
    : QDialog(parent), m_location(loc), m_manager(manager)
{
    setWindowTitle(tr("Location Info"));
    setMinimumWidth(600); // Make dialog twice as wide (adjust as needed)

    QVBoxLayout* outerLayout = new QVBoxLayout(this);
    QHBoxLayout* mainLayout = new QHBoxLayout();
    outerLayout->addLayout(mainLayout);
    QVBoxLayout* leftLayout = new QVBoxLayout();
    QVBoxLayout* rightLayout = new QVBoxLayout();
    mainLayout->addLayout(leftLayout, 3); // main fields wider
    mainLayout->addLayout(rightLayout, 2); // edges area

    // ID and coords (read-only)
    auto locationTypeToString = [](tge::domain::LocationType t) -> QString {
        switch (t) {
        case tge::domain::LocationType::Start:   return QStringLiteral("Start");
        case tge::domain::LocationType::Regular: return QStringLiteral("Regular");
        case tge::domain::LocationType::Service: return QStringLiteral("Service");
        case tge::domain::LocationType::Finish:  return QStringLiteral("Finish");
        }
        return QStringLiteral("Unknown");
    };
    QString idType = QString("Loc id: %1 (%2): (%3,%4)").arg(loc->id)
      .arg(locationTypeToString(loc->type))
      .arg(loc->coordX).arg(loc->coordY);
    QLabel* idLabel = new QLabel(idType, this);
    leftLayout->addWidget(idLabel);

    // Color palette UI
    setupColorPaletteUI(leftLayout);

    // Label
    m_labelEdit = new QLineEdit(loc->label, this);
    leftLayout->addWidget(m_labelEdit);

    // Description tabs
    QHBoxLayout* descBtnLayout = new QHBoxLayout;
    m_addDescBtn = new QPushButton("+", this);
    m_removeDescBtn = new QPushButton("-", this);
    descBtnLayout->addWidget(m_addDescBtn);
    descBtnLayout->addWidget(m_removeDescBtn);
    leftLayout->addLayout(descBtnLayout);
    m_descTabs = new QTabWidget(this);
    leftLayout->addWidget(m_descTabs);
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
    rightLayout->addWidget(new QLabel(tr("Edges (incoming/outgoing):"), this));
    m_edgeListWidget = new QListWidget(this);
    rightLayout->addWidget(m_edgeListWidget);
    populateEdgeList();
    connect(m_edgeListWidget, &QListWidget::itemClicked, this, &LocationDialog::onEdgeItemClicked);

    // Buttons (below all content)
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    outerLayout->addWidget(buttons);
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
    EdgeDialog dlg(edge,
                   fromLoc,
                   toLoc,
                   game.globalVariables,
                   game.infoDisplayItems,
                   this);
    if (dlg.exec() == QDialog::Accepted) {
        edge.optionText = dlg.optionText();
        edge.transitionText = dlg.transitionText();
        edge.condition = dlg.conditionText();
        edge.variableSettings = dlg.variableSettings();
        edge.infoDisplayItemSettings = dlg.infoDisplayItemSettings();
        edge.color = dlg.edgeColor();
        populateEdgeList();
    }
}

void LocationDialog::setupColorPaletteUI(QVBoxLayout* layout) {
    QGridLayout* grid = new QGridLayout();
    m_colorButtonGroup = new QButtonGroup(this);
    m_colorButtonGroup->setExclusive(true);
    m_colorButtons.clear();
    // 15 palette colors + 1 no color
    for (int i = 0; i < tge::domain::LOCATION_COLOR_COUNT + 1; ++i) {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(32, 32);
        if (i < tge::domain::LOCATION_COLOR_COUNT) {
            btn->setStyleSheet(QString("background-color: %1;").arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            btn->setToolTip(tr("Color %1").arg(i+1));
        } else {
            btn->setStyleSheet("background: none; border: 1px dashed #aaa;");
            btn->setText("X");
            btn->setToolTip(tr("No color"));
        }
        m_colorButtonGroup->addButton(btn, i);
        m_colorButtons.append(btn);
        grid->addWidget(btn, i / 8, i % 8);
    }
    layout->addLayout(grid);
    connect(m_colorButtonGroup, &QButtonGroup::idClicked, this, &LocationDialog::onColorButtonClicked);
    updateColorSelection();
}

void LocationDialog::onColorButtonClicked(int id) {
    if (id < tge::domain::LOCATION_COLOR_COUNT) {
        m_location->color = id;
    } else {
        m_location->color = tge::domain::LOCATION_COLOR_NONE;
    }
    updateColorSelection();
}

void LocationDialog::updateColorSelection() {
    int sel = m_location->color;
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        if ((sel == -1 && i == tge::domain::LOCATION_COLOR_COUNT) || (sel == i)) {
            m_colorButtons[i]->setStyleSheet(m_colorButtons[i]->styleSheet() + "; border: 3px solid #333;");
        } else {
            // Reset border
            if (i < tge::domain::LOCATION_COLOR_COUNT)
                m_colorButtons[i]->setStyleSheet(QString("background-color: %1; border: 1px solid #aaa;").arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            else
                m_colorButtons[i]->setStyleSheet("background: none; border: 1px dashed #aaa;");
        }
    }
}
