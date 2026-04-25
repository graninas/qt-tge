#include "mainwindow.h"
#include "ui_mainwindow.h"
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

    graphWidget = new GraphWidget(this);
    setCentralWidget(graphWidget);

    static UiModel model = UiModel::makeTestGame();
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
