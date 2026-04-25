#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "tge/domain.h"

class EdgeListItemWidget : public QWidget {
    Q_OBJECT
public:
    EdgeListItemWidget(const tge::domain::EdgeDef& edge, int thisLocId, QWidget* parent = nullptr);
    QPushButton* deleteButton() const { return m_deleteBtn; }
    int edgeId() const { return m_edgeId; }
signals:
    void deleteRequested(int edgeId);
private:
    int m_edgeId;
    QPushButton* m_deleteBtn;
};
