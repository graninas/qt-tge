#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"
#include "globalvariablesdialog.h"
#include "infodisplayitemsdialog.h"
#include "playerwidget.h"

#include <QLabel>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    graphWidget = new GraphWidget(central);
    playerWidget = new PlayerWidget(central);
    playerWidget->setVisible(false);
    playerWidget->setMinimumHeight(800);

    mainLayout->addWidget(graphWidget, 2);
    mainLayout->addWidget(playerWidget, 1);
    setCentralWidget(central);

    static UiModel staticModel = UiModel::makeTestGame();
    model = &staticModel;
    model->editorState.setMode(tge::editor::EditingMode::StaticModel);
    graphWidget->setModel(model);
    connect(graphWidget, &GraphWidget::selectionChanged, this, &MainWindow::updateSelectionSummary);

    // Toolbox setup
    QToolBar *toolBar = new QToolBar("Toolbox", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);

    globalVariablesButton = new QToolButton(this);
    globalVariablesButton->setText("Global Variables");
    globalVariablesButton->setToolTip("Edit global variable definitions");
    toolBar->addWidget(globalVariablesButton);
    connect(globalVariablesButton, &QToolButton::clicked, this, &MainWindow::onEditGlobalVariables);

    infoDisplayItemsButton = new QToolButton(this);
    infoDisplayItemsButton->setText("Info Display");
    infoDisplayItemsButton->setToolTip("Edit game info display items");
    toolBar->addWidget(infoDisplayItemsButton);
    connect(infoDisplayItemsButton, &QToolButton::clicked, this, &MainWindow::onEditInfoDisplayItems);

    playerButton = new QToolButton(this);
    playerButton->setText("Player");
    playerButton->setToolTip("Toggle player mode");
    playerButton->setCheckable(true);
    toolBar->addWidget(playerButton);
    connect(playerButton, &QToolButton::toggled, this, &MainWindow::onPlayerToggled);

    selectionSummaryLabel = new QLabel(this);
    selectionSummaryLabel->setToolTip("Selected locations and edges");
    toolBar->addWidget(selectionSummaryLabel);
    updateSelectionSummary(model->selectedLocationIds.size(), model->selectedEdgeIds.size());
    applyEditorModeUiState();
}

void MainWindow::onEditGlobalVariables()
{
    if (!model) {
        return;
    }

    GlobalVariablesDialog dlg(model->gameDef.globalVariables, model->editorState.capabilities, this);
    if (dlg.exec() == QDialog::Accepted) {
        graphWidget->viewport()->update();
    }
}

void MainWindow::onEditInfoDisplayItems()
{
    if (!model) {
        return;
    }

    InfoDisplayItemsDialog dlg(model->gameDef.infoDisplayItems, model->editorState.capabilities, this);
    if (dlg.exec() == QDialog::Accepted) {
        graphWidget->viewport()->update();
    }
}

void MainWindow::onPlayerToggled(bool enabled)
{
    if (!model) {
        return;
    }

    if (enabled) {
        playerWidget->setGameDef(&model->gameDef);
        QString startError;
        if (!playerWidget->startSession(&startError)) {
            QMessageBox::warning(this,
                                 tr("Player initialization failed"),
                                 startError.isEmpty() ? tr("Unknown error") : startError);
            playerButton->blockSignals(true);
            playerButton->setChecked(false);
            playerButton->blockSignals(false);
            return;
        }
        model->editorState.setMode(tge::editor::EditingMode::Player);
    } else {
        playerWidget->clearSession();
        model->editorState.setMode(tge::editor::EditingMode::StaticModel);
    }

    applyEditorModeUiState();
    graphWidget->viewport()->update();
}

void MainWindow::applyEditorModeUiState()
{
    if (!model) {
        return;
    }

    const bool playerMode = (model->editorState.mode == tge::editor::EditingMode::Player);
    if (playerWidget) {
        playerWidget->setVisible(playerMode);
    }

    if (globalVariablesButton) {
        globalVariablesButton->setEnabled(true);
    }

    if (infoDisplayItemsButton) {
        infoDisplayItemsButton->setEnabled(true);
    }
}

void MainWindow::updateSelectionSummary(int locationCount, int edgeCount)
{
    if (!selectionSummaryLabel) {
        return;
    }

    selectionSummaryLabel->setText(QString("Selected: L %1, E %2").arg(locationCount).arg(edgeCount));
}

MainWindow::~MainWindow()
{
    delete ui;
}
