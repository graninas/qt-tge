#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"
#include "globalvariablesdialog.h"
#include "infodisplayitemsdialog.h"

#include <QLabel>
#include <QToolBar>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    graphWidget = new GraphWidget(this);
    setCentralWidget(graphWidget);

    static UiModel staticModel = UiModel::makeTestGame();
    model = &staticModel;
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

    selectionSummaryLabel = new QLabel(this);
    selectionSummaryLabel->setToolTip("Selected locations and edges");
    toolBar->addWidget(selectionSummaryLabel);
    updateSelectionSummary(model->selectedLocationIds.size(), model->selectedEdgeIds.size());
}

void MainWindow::onEditGlobalVariables()
{
    if (!model) {
        return;
    }

    GlobalVariablesDialog dlg(model->gameDef.globalVariables, this);
    if (dlg.exec() == QDialog::Accepted) {
        graphWidget->viewport()->update();
    }
}

void MainWindow::onEditInfoDisplayItems()
{
    if (!model) {
        return;
    }

    InfoDisplayItemsDialog dlg(model->gameDef.infoDisplayItems, this);
    if (dlg.exec() == QDialog::Accepted) {
        graphWidget->viewport()->update();
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
