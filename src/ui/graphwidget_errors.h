#pragma once
#include <QString>
#include <QStringList>
#include <QPoint>
#include <QTimer>
#include <QPainter>

class GraphWidget;

namespace graphwidget_errors {

void showErrorMessage(GraphWidget* widget, const QString& msg, const QPoint& pos);
void clearErrorMessage(GraphWidget* widget);
QStringList wrapErrorMessage(const QString& msg, int maxLineLen);
void drawErrorMessage(const GraphWidget* widget, QPainter& painter);

}
