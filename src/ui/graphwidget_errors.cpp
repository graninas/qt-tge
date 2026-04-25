#include "graphwidget_errors.h"
#include "graphwidget.h"
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QColor>
#include <algorithm>

namespace graphwidget_errors {

void showErrorMessage(GraphWidget* widget, const QString& msg, const QPoint& pos) {
    widget->errorMessage = msg;
    widget->errorCursorPos = pos;
    widget->errorTimer.start(3500); // Show for 2.5 seconds
    widget->viewport()->update();
}

void clearErrorMessage(GraphWidget* widget) {
    widget->errorMessage.clear();
    widget->errorTimer.stop();
    widget->viewport()->update();
}

QStringList wrapErrorMessage(const QString& msg, int maxLineLen) {
    QStringList lines;
    QString currentLine;
    const QStringList words = msg.split(' ');
    for (const QString& word : words) {
        if (currentLine.length() + word.length() + 1 > maxLineLen && !currentLine.isEmpty()) {
            lines << currentLine;
            currentLine.clear();
        }
        if (!currentLine.isEmpty()) currentLine += ' ';
        currentLine += word;
    }
    if (!currentLine.isEmpty()) lines << currentLine;
    return lines;
}

void drawErrorMessage(const GraphWidget* widget, QPainter& painter) {
    if (widget->errorMessage.isEmpty()) return;
    painter.save();
    painter.resetTransform();
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QFontMetrics fm(font);
    int pad = 8;
    QStringList lines = wrapErrorMessage(widget->errorMessage, 48);
    int width = 0;
    for (const QString& line : lines) width = std::max(width, fm.horizontalAdvance(line));
    int height = lines.size() * fm.height();
    QRect rect(widget->errorCursorPos.x() + 25, widget->errorCursorPos.y() + 20, width + 2*pad, height + 2*pad);
    QColor bgColor(255, 200, 200);
    QColor borderColor(180, 40, 40);
    QColor shadowColor(0, 0, 0, 60);
    QRect shadowRect = rect.translated(4, 4);
    painter.setBrush(shadowColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(shadowRect, 10, 10);
    painter.setBrush(bgColor);
    painter.setPen(borderColor);
    painter.drawRoundedRect(rect, 10, 10);
    painter.setPen(Qt::black);
    int y = rect.top() + pad + fm.ascent();
    for (const QString& line : lines) {
        painter.drawText(rect.left() + pad, y, line);
        y += fm.height();
    }
    painter.restore();
}

} // namespace graphwidget_errors
