#include "mainwindow.h"
#include "ui/ui_mainwindow.h"
#include "graphwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Add the graph widget to the central widget
    graphWidget = new GraphWidget(this);
    setCentralWidget(graphWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
