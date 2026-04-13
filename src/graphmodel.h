#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "tge/domain.h"
#include <QVector>
#include <QColor>
#include <QString>

// Simple model for the UI to hold a static graph
class GraphModel {
public:
    QVector<tge::domain::LocationDef> locations;
    QVector<tge::domain::EdgeDef> edges;

    // Helper to create a static test graph (3 nodes, 2 edges)
    static GraphModel makeTestGraph();
};

#endif // GRAPHMODEL_H
