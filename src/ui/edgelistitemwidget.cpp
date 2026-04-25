#include "edgelistitemwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>

EdgeListItemWidget::EdgeListItemWidget(const tge::domain::EdgeDef& edge, int thisLocId, QWidget* parent)
    : QWidget(parent), m_edgeId(edge.id)
{
    const bool outgoing = (edge.fromLocation == thisLocId);
    const int adjLocId = outgoing ? edge.toLocation : edge.fromLocation;

    QString tooltip = tr("Edge #%1\n%2 -> %3")
        .arg(edge.id)
        .arg(edge.fromLocation)
        .arg(edge.toLocation);
    tooltip += tr("\nOption: %1")
        .arg(edge.optionText.trimmed().isEmpty() ? tr("(empty)") : edge.optionText);
    tooltip += tr("\nTransition: %1")
        .arg(edge.transitionText.trimmed().isEmpty() ? tr("(empty)") : edge.transitionText);
    tooltip += tr("\nCondition: %1")
        .arg(edge.condition.trimmed().isEmpty() ? tr("(always available)") : edge.condition);

    setToolTip(tooltip);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    // Bold edge id
    QLabel* idLabel = new QLabel(QString::number(edge.id), this);
    QFont boldFont = idLabel->font();
    boldFont.setBold(true);
    idLabel->setFont(boldFont);
    layout->addWidget(idLabel);
    // Separator
    layout->addWidget(new QLabel("|", this));
    // Location ids and arrow
    QString arrow = outgoing ? QString::fromUtf8("\u2192") : QString::fromUtf8("\u2190");
    QLabel* locLabel = new QLabel(QString("%1 %2 %3").arg(thisLocId).arg(arrow).arg(adjLocId), this);
    locLabel->setToolTip(tooltip);
    layout->addWidget(locLabel);
    // Separator
    layout->addWidget(new QLabel("|", this));
    // Italic option text
    QLabel* optLabel = new QLabel(edge.optionText.isEmpty() ? tr("(no label)") : edge.optionText, this);
    optLabel->setToolTip(tooltip);
    QFont italicFont = optLabel->font();
    italicFont.setItalic(true);
    optLabel->setFont(italicFont);
    layout->addWidget(optLabel);
    // Spacer
    layout->addStretch();
    // Delete button
    m_deleteBtn = new QPushButton(QString::fromUtf8("\u2716"), this); // ✖
    m_deleteBtn->setToolTip(tr("Delete edge"));
    m_deleteBtn->setFixedWidth(28);
    layout->addWidget(m_deleteBtn);
    connect(m_deleteBtn, &QPushButton::clicked, this, [this]() { emit deleteRequested(m_edgeId); });
}
