#include "mainwindow.h"
#include "ui/ui_mainwindow.h"
#include "graphwidget.h"
#include "graphmodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Add the graph widget to the central widget
    graphWidget = new GraphWidget(this);
    setCentralWidget(graphWidget);
    // Create and set the static test graph
    static GraphModel model = GraphModel::makeTestGraph();
    graphWidget->setModel(&model);
}

MainWindow::~MainWindow()
{
    delete ui;
}
