#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
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
    void onEditGlobalVariables();
    void onEditInfoDisplayItems();
    void updateSelectionSummary(int locationCount, int edgeCount);

private:
    Ui::MainWindow *ui;
    GraphWidget *graphWidget; // Pointer to the graph widget
    QToolButton *globalVariablesButton; // Button for global variables editor
    QToolButton *infoDisplayItemsButton; // Button for info display items editor
    QLabel *selectionSummaryLabel = nullptr;
    UiModel *model = nullptr;
};

#endif // MAINWINDOW_H
