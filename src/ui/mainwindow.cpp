#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "graphwidget.h"
#include "globalvariablesdialog.h"
#include "infodisplayitemsdialog.h"

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

    // Toolbox setup
    QToolBar *toolBar = new QToolBar("Toolbox", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);

    newLocationButton = new QToolButton(this);
    newLocationButton->setText("New Location");
    newLocationButton->setCheckable(true);
    newLocationButton->setToolTip("Add new location");
    toolBar->addWidget(newLocationButton);
    connect(newLocationButton, &QToolButton::clicked, this, &MainWindow::onNewLocationMode);

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
}

void MainWindow::onNewLocationMode()
{
    const bool enabled = newLocationButton->isChecked();
    graphWidget->setNewLocationMode(enabled);
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

MainWindow::~MainWindow()
{
    delete ui;
}
