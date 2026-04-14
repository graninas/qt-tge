#include "mainwindow.h"
#include "ui/ui_mainwindow.h"
#include "graphwidget.h"
#include "gui_model.h"
#include <QToolBar>
#include <QToolButton>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Add the graph widget to the central widget
    graphWidget = new GraphWidget(this);
    setCentralWidget(graphWidget);
    // Create and set the static test graph
    static UiModel model = UiModel::makeTestGraph();
    graphWidget->setModel(&model);

    // Toolbox setup
    QToolBar *toolBar = new QToolBar("Toolbox", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);
    newLocationButton = new QToolButton(this);
    newLocationButton->setText("New Location");
    newLocationButton->setCheckable(true);
    newLocationButton->setToolTip("Add new location");
    toolBar->addWidget(newLocationButton);
    connect(newLocationButton, &QToolButton::clicked, this, &MainWindow::onNewLocationMode);
}

void MainWindow::onNewLocationMode()
{
    bool enabled = newLocationButton->isChecked();
    graphWidget->setNewLocationMode(enabled);
}

MainWindow::~MainWindow()
{
    delete ui;
}
