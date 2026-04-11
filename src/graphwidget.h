#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QGraphicsView>
#include <QColor>
#include <QPointF>

class GraphWidget : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    struct GridSettings {
        double scale = 50.0; // Cell size in pixels
        QColor color = QColor(220, 220, 220); // Light gray
        QPointF center = QPointF(0, 0); // Center of the view in grid coordinates
    };

    GridSettings gridSettings;
};

#endif // GRAPHWIDGET_H
