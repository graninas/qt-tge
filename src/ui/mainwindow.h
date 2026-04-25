#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>

#include "gui_model.h"

namespace Ui {
class MainWindow;
}

class GraphWidget; // Forward declaration of GraphWidget class

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewLocationMode();
    void onEditGlobalVariables();

private:
    Ui::MainWindow *ui;
    GraphWidget *graphWidget; // Pointer to the graph widget
    QToolButton *newLocationButton; // Button for new location mode
    QToolButton *globalVariablesButton; // Button for global variables editor
    UiModel *model = nullptr;
};

#endif // MAINWINDOW_H
