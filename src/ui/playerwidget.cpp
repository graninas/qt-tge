#include "playerwidget.h"

#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>

using tge::domain::InfoDisplayItemMode;
using tge::player::runtime::CurrentLocation;
using tge::player::runtime::CurrentTransition;
using tge::player::runtime::FinishLocation;
using tge::player::runtime::TransitionOption;

PlayerWidget::PlayerWidget(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
    updateUi();
}

void PlayerWidget::setGameDef(const tge::domain::GameDef *gameDef)
{
    m_gameDef = gameDef;
}

bool PlayerWidget::startSession(QString *error)
{
    m_engine.reset();
    m_currentStep.reset();
    m_optionRows.clear();
    m_confirmationCount = 0;

    if (!m_gameDef) {
        const QString message = tr("Game definition is not set.");
        if (error) {
            *error = message;
        }
        updateUi();
        return false;
    }

    m_engine.emplace(*m_gameDef, tge::player::GameMode::Debug);
    if (m_engine->hasError()) {
        const QString message = m_engine->error();
        if (error) {
            *error = message;
        }
        m_engine.reset();
        updateUi();
        return false;
    }

    const std::optional<CurrentLocation> startLocation = m_engine->start();
    if (!startLocation.has_value()) {
        const QString message = tr("Engine could not provide start location.");
        if (error) {
            *error = message;
        }
        m_engine.reset();
        updateUi();
        return false;
    }

    m_currentStep = tge::player::runtime::StepResult(startLocation.value());
    updateUi();
    return true;
}

void PlayerWidget::clearSession()
{
    m_engine.reset();
    m_currentStep.reset();
    m_optionRows.clear();
    m_confirmationCount = 0;
    updateUi();
}

void PlayerWidget::onStartRestartClicked()
{
    QString error;
    if (!startSession(&error)) {
        QMessageBox::warning(this, tr("Player start failed"), error.isEmpty() ? tr("Unknown error") : error);
    }
}

void PlayerWidget::onConfirmClicked()
{
    if (!m_engine.has_value() || !m_currentStep.has_value()) {
        return;
    }

    if (std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        const int selectedRow = m_optionsList->currentRow();
        if (selectedRow < 0 || selectedRow >= m_optionRows.size()) {
            QMessageBox::information(this,
                                     tr("Select transition"),
                                     tr("Choose an available transition option before confirming."));
            return;
        }

        const OptionRow &selected = m_optionRows[selectedRow];
        if (!selected.available) {
            QMessageBox::information(this,
                                     tr("Transition unavailable"),
                                     tr("The selected transition is currently unavailable."));
            return;
        }

        const auto &location = std::get<CurrentLocation>(m_currentStep.value());
        const std::optional<CurrentTransition> transition = m_engine->choose(location, selected.edgeId);
        if (!transition.has_value()) {
            QMessageBox::warning(this,
                                 tr("Transition failed"),
                                 tr("Engine could not build transition for the selected edge."));
            return;
        }

        m_currentStep = tge::player::runtime::StepResult(transition.value());
        ++m_confirmationCount;
        updateUi();
        return;
    }

    if (std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        const auto &transition = std::get<CurrentTransition>(m_currentStep.value());
        m_currentStep = m_engine->step(transition);
        ++m_confirmationCount;
        updateUi();
        return;
    }

    updateUi();
}

void PlayerWidget::onOptionSelectionChanged()
{
    updateConfirmButtonState();
}

void PlayerWidget::buildUi()
{
    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    QVBoxLayout *left = new QVBoxLayout();
    QVBoxLayout *middle = new QVBoxLayout();
    QVBoxLayout *right = new QVBoxLayout();

    m_stateLabel = new QLabel(this);
    m_confirmationsLabel = new QLabel(this);

    QGroupBox *messageBox = new QGroupBox(tr("Current message"), this);
    QVBoxLayout *messageLayout = new QVBoxLayout(messageBox);
    m_messageEdit = new QPlainTextEdit(messageBox);
    m_messageEdit->setReadOnly(true);
    m_messageEdit->setMinimumHeight(120);
    messageLayout->addWidget(m_messageEdit);

    QGroupBox *optionsBox = new QGroupBox(tr("Transition options"), this);
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsBox);
    m_optionsList = new QListWidget(optionsBox);
    optionsLayout->addWidget(m_optionsList);

    QHBoxLayout *controls = new QHBoxLayout();
    m_startRestartButton = new QPushButton(tr("Restart player"), this);
    m_confirmButton = new QPushButton(tr("Confirm"), this);
    controls->addWidget(m_startRestartButton);
    controls->addWidget(m_confirmButton);

    left->addWidget(m_stateLabel);
    left->addWidget(m_confirmationsLabel);
    left->addWidget(messageBox, 2);
    left->addWidget(optionsBox, 2);
    left->addLayout(controls);

    QGroupBox *pendingBox = new QGroupBox(tr("Pending transition changes"), this);
    QVBoxLayout *pendingLayout = new QVBoxLayout(pendingBox);
    m_pendingEdit = new QPlainTextEdit(pendingBox);
    m_pendingEdit->setReadOnly(true);
    pendingLayout->addWidget(m_pendingEdit);

    QGroupBox *debugBox = new QGroupBox(tr("Debug messages"), this);
    QVBoxLayout *debugLayout = new QVBoxLayout(debugBox);
    m_debugEdit = new QPlainTextEdit(debugBox);
    m_debugEdit->setReadOnly(true);
    debugLayout->addWidget(m_debugEdit);

    middle->addWidget(pendingBox, 1);
    middle->addWidget(debugBox, 1);

    QGroupBox *infoDisplayBox = new QGroupBox(tr("Info display state"), this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoDisplayBox);
    m_infoDisplayTable = new QTableWidget(infoDisplayBox);
    m_infoDisplayTable->setColumnCount(7);
    m_infoDisplayTable->setHorizontalHeaderLabels(
        QStringList() << tr("ID") << tr("Label") << tr("Value") << tr("Visible") << tr("Priority") << tr("Show") << tr("Mode"));
    m_infoDisplayTable->horizontalHeader()->setStretchLastSection(true);
    m_infoDisplayTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_infoDisplayTable->verticalHeader()->setVisible(false);
    m_infoDisplayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_infoDisplayTable->setSelectionMode(QAbstractItemView::NoSelection);
    infoLayout->addWidget(m_infoDisplayTable);

    QGroupBox *variablesBox = new QGroupBox(tr("Global variables"), this);
    QVBoxLayout *variablesLayout = new QVBoxLayout(variablesBox);
    m_variablesTable = new QTableWidget(variablesBox);
    m_variablesTable->setColumnCount(3);
    m_variablesTable->setHorizontalHeaderLabels(QStringList() << tr("Index") << tr("Name") << tr("Value"));
    m_variablesTable->horizontalHeader()->setStretchLastSection(true);
    m_variablesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_variablesTable->verticalHeader()->setVisible(false);
    m_variablesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_variablesTable->setSelectionMode(QAbstractItemView::NoSelection);
    variablesLayout->addWidget(m_variablesTable);

    right->addWidget(infoDisplayBox, 2);
    right->addWidget(variablesBox, 1);

    root->addLayout(left, 3);
    root->addLayout(middle, 2);
    root->addLayout(right, 3);

    connect(m_startRestartButton, &QPushButton::clicked, this, &PlayerWidget::onStartRestartClicked);
    connect(m_confirmButton, &QPushButton::clicked, this, &PlayerWidget::onConfirmClicked);
    connect(m_optionsList, &QListWidget::currentRowChanged, this, &PlayerWidget::onOptionSelectionChanged);
}

void PlayerWidget::updateUi()
{
    m_stateLabel->setText(buildStepTitle());
    m_confirmationsLabel->setText(tr("Confirmed actions: %1").arg(m_confirmationCount));

    updateStepSections();
    updateInfoDisplayTable();
    updateVariablesTable();
    updateConfirmButtonState();
}

void PlayerWidget::updateConfirmButtonState()
{
    const bool hasSession = m_engine.has_value() && m_currentStep.has_value();
    bool canConfirm = false;
    QString confirmText = tr("Confirm");

    if (hasSession && std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        confirmText = tr("Confirm selected transition");
        const int row = m_optionsList->currentRow();
        canConfirm = row >= 0 && row < m_optionRows.size() && m_optionRows[row].available;
    } else if (hasSession && std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        confirmText = tr("Apply transition step");
        canConfirm = true;
    }

    m_confirmButton->setText(confirmText);
    m_confirmButton->setEnabled(canConfirm);
}

void PlayerWidget::updateStepSections()
{
    m_messageEdit->setPlainText(buildStepMessage());

    const QStringList debugMessages = buildStepDebugMessages();
    m_debugEdit->setPlainText(debugMessages.isEmpty() ? tr("No debug messages") : debugMessages.join("\n"));

    m_pendingEdit->clear();
    m_optionsList->clear();
    m_optionRows.clear();

    if (!m_currentStep.has_value()) {
        m_pendingEdit->setPlainText(tr("No pending changes"));
        return;
    }

    if (std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        const auto &location = std::get<CurrentLocation>(m_currentStep.value());
        updateOptionsForLocation(location);
        m_pendingEdit->setPlainText(tr("Pending changes appear after transition selection."));
        return;
    }

    if (std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        const auto &transition = std::get<CurrentTransition>(m_currentStep.value());
        m_pendingEdit->setPlainText(buildPendingSummary(transition));
        return;
    }

    m_pendingEdit->setPlainText(tr("Game finished. No pending changes."));
}

void PlayerWidget::updateVariablesTable()
{
    m_variablesTable->setRowCount(0);
    if (!m_engine.has_value()) {
        return;
    }

    const auto &variables = m_engine->state().variables;
    m_variablesTable->setRowCount(static_cast<int>(variables.size()));

    for (int row = 0; row < static_cast<int>(variables.size()); ++row) {
        const auto &variable = variables[static_cast<size_t>(row)];
        const QString index = variable.def ? variable.def->index : tr("?");
        const QString name = variable.def ? variable.def->name : tr("Unknown");

        m_variablesTable->setItem(row, 0, new QTableWidgetItem(index));
        m_variablesTable->setItem(row, 1, new QTableWidgetItem(name));
        m_variablesTable->setItem(row, 2, new QTableWidgetItem(variable.value));
    }
}

void PlayerWidget::updateInfoDisplayTable()
{
    m_infoDisplayTable->setRowCount(0);
    if (!m_engine.has_value()) {
        return;
    }

    std::vector<const tge::player::InfoDisplayItemState*> sortedItems;
    sortedItems.reserve(m_engine->state().infoDisplayItems.size());
    for (const auto &item : m_engine->state().infoDisplayItems) {
        sortedItems.push_back(&item);
    }

    std::stable_sort(sortedItems.begin(), sortedItems.end(), [](const auto *lhs, const auto *rhs) {
        return lhs->priority < rhs->priority;
    });

    m_infoDisplayTable->setRowCount(static_cast<int>(sortedItems.size()));
    for (int row = 0; row < static_cast<int>(sortedItems.size()); ++row) {
        const auto *item = sortedItems[static_cast<size_t>(row)];
        const QString mode = (item->def && item->def->mode == InfoDisplayItemMode::Debug) ? tr("Debug") : tr("Actual");

        m_infoDisplayTable->setItem(row, 0, new QTableWidgetItem(item->def ? QString::number(item->def->id) : tr("?")));
        m_infoDisplayTable->setItem(row, 1, new QTableWidgetItem(item->def ? item->def->label : tr("Unknown")));
        m_infoDisplayTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(item->value)));
        m_infoDisplayTable->setItem(row, 3, new QTableWidgetItem(item->visible ? tr("yes") : tr("no")));
        m_infoDisplayTable->setItem(row, 4, new QTableWidgetItem(QString::number(item->priority)));
        m_infoDisplayTable->setItem(row, 5, new QTableWidgetItem(item->showFormulaValue ? tr("yes") : tr("no")));
        m_infoDisplayTable->setItem(row, 6, new QTableWidgetItem(mode));
    }
}

void PlayerWidget::updateOptionsForLocation(const CurrentLocation &location)
{
    const QSignalBlocker signalBlocker(m_optionsList);
    m_optionsList->clear();
    m_optionRows.clear();

    int firstAvailableRow = -1;

    for (const TransitionOption &option : location.options) {
        if (!option.edge || !option.edge->def) {
            continue;
        }

        const QString status = option.isAvailable ? tr("available") : tr("blocked");
        const QString text = QString("#%1  %2 -> %3  |  %4  |  %5")
                                 .arg(option.edge->def->id)
                                 .arg(option.edge->def->fromLocation)
                                 .arg(option.edge->def->toLocation)
                                 .arg(option.edge->def->optionText)
                                 .arg(status);

        QListWidgetItem *item = new QListWidgetItem(text, m_optionsList);
        if (!option.isAvailable) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        }

        OptionRow row;
        row.edgeId = option.edge->def->id;
        row.available = option.isAvailable;
        m_optionRows.append(row);

        if (firstAvailableRow < 0 && option.isAvailable) {
            firstAvailableRow = m_optionRows.size() - 1;
        }
    }

    if (firstAvailableRow >= 0) {
        m_optionsList->setCurrentRow(firstAvailableRow);
    }
}

QString PlayerWidget::buildStepTitle() const
{
    if (!m_currentStep.has_value()) {
        return tr("Player: no active session");
    }

    if (std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        const auto &location = std::get<CurrentLocation>(m_currentStep.value());
        if (location.location && location.location->def) {
            return tr("State: location #%1 (%2)")
                .arg(location.location->def->id)
                .arg(location.location->def->type == tge::domain::LocationType::Service ? tr("service") : tr("regular/start"));
        }
        return tr("State: location");
    }

    if (std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        const auto &transition = std::get<CurrentTransition>(m_currentStep.value());
        return tr("State: transition #%1 pending")
            .arg((transition.edge && transition.edge->def) ? transition.edge->def->id : -1);
    }

    const auto &finish = std::get<FinishLocation>(m_currentStep.value());
    return tr("State: finish #%1 reached")
        .arg((finish.location && finish.location->def) ? finish.location->def->id : -1);
}

QString PlayerWidget::buildStepMessage() const
{
    if (!m_currentStep.has_value()) {
        return tr("Press 'Restart player' to initialize Debug play mode.");
    }

    if (std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        return std::get<CurrentLocation>(m_currentStep.value()).description;
    }

    if (std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        const auto &transition = std::get<CurrentTransition>(m_currentStep.value());
        return transition.transitionText;
    }

    return std::get<FinishLocation>(m_currentStep.value()).description;
}

QStringList PlayerWidget::buildStepDebugMessages() const
{
    if (!m_currentStep.has_value()) {
        return {};
    }

    QStringList messages;

    if (std::holds_alternative<CurrentLocation>(m_currentStep.value())) {
        const auto &location = std::get<CurrentLocation>(m_currentStep.value());
        for (const QString &message : location.debugMessages) {
            messages.append(message);
        }
    } else if (std::holds_alternative<CurrentTransition>(m_currentStep.value())) {
        const auto &transition = std::get<CurrentTransition>(m_currentStep.value());
        for (const QString &message : transition.debugMessages) {
            messages.append(message);
        }
    } else {
        const auto &finish = std::get<FinishLocation>(m_currentStep.value());
        for (const QString &message : finish.debugMessages) {
            messages.append(message);
        }
    }

    return messages;
}

QString PlayerWidget::buildPendingSummary(const CurrentTransition &transition) const
{
    QStringList lines;

    lines.append(tr("Variable changes: %1").arg(transition.pendingVariableChanges.size()));
    for (const auto &change : transition.pendingVariableChanges) {
        const QString variableName = (change.def ? change.def->name : tr("Unknown"));
        const QString variableIndex = (change.def ? change.def->index : tr("?"));
        lines.append(tr("- %1 (%2) -> %3").arg(variableName).arg(variableIndex).arg(change.newValue));
    }

    lines.append(tr("Info display changes: %1").arg(transition.pendingInfoDisplayItemChanges.size()));
    for (const auto &change : transition.pendingInfoDisplayItemChanges) {
        lines.append(tr("- item #%1").arg(change.itemIndex));
        if (change.changePriority) {
            lines.append(tr("    priority -> %1").arg(change.newPriority));
        }
        if (change.changeVisibility) {
            lines.append(tr("    visibility -> %1").arg(change.newVisibility ? tr("shown") : tr("hidden")));
        }
        if (change.changeShowValue) {
            lines.append(tr("    show value -> %1").arg(change.newShowValue ? tr("yes") : tr("no")));
        }
        if (change.changeValue) {
            lines.append(tr("    value -> %1").arg(QString::fromStdString(change.newValue)));
        }
    }

    return lines.join("\n");
}
