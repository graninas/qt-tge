#include "edgedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QGridLayout>
#include "tge/domain.h"
#include "tge/formula/parser.h"
#include "graphwidget_helpers.h"

EdgeDialog::EdgeDialog(const tge::domain::EdgeDef& edge,
                       const tge::domain::LocationDef& fromLoc,
                       const tge::domain::LocationDef& toLoc,
                       QWidget* parent)
        : QDialog(parent),
            m_colorButtonGroup(nullptr),
            m_selectedColor(edge.color)
{
    setWindowTitle(tr("Edge Info"));
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Edge ID
    m_idLabel = new QLabel(tr("Edge ID: %1").arg(edge.id), this);
    layout->addWidget(m_idLabel);

    // From location
    m_fromLabel = new QLabel(tr("From: %1 (%2)").arg(fromLoc.id).arg(fromLoc.label), this);
    layout->addWidget(m_fromLabel);

    // To location
    m_toLabel = new QLabel(tr("To: %1 (%2)").arg(toLoc.id).arg(toLoc.label), this);
    layout->addWidget(m_toLabel);

    // Custom edge color
    layout->addWidget(new QLabel(tr("Color:"), this));
    QGridLayout* colorGrid = new QGridLayout();
    m_colorButtonGroup = new QButtonGroup(this);
    m_colorButtonGroup->setExclusive(true);
    m_colorButtons.clear();
    for (int i = 0; i < tge::domain::LOCATION_COLOR_COUNT + 1; ++i) {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(28, 28);
        if (i < tge::domain::LOCATION_COLOR_COUNT) {
            btn->setStyleSheet(QString("background-color: %1;").arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            btn->setToolTip(tr("Color %1").arg(i + 1));
        } else {
            btn->setStyleSheet("background: none; border: 1px dashed #aaa;");
            btn->setText("X");
            btn->setToolTip(tr("Default edge color"));
        }
        m_colorButtonGroup->addButton(btn, i);
        m_colorButtons.append(btn);
        colorGrid->addWidget(btn, i / 8, i % 8);
    }
    layout->addLayout(colorGrid);
    connect(m_colorButtonGroup, &QButtonGroup::idClicked, this, &EdgeDialog::onColorButtonClicked);
    updateColorSelection();

    // Option text
    layout->addWidget(new QLabel(tr("Option Text:"), this));
    m_optionEdit = new QTextEdit(edge.optionText, this);
    m_optionEdit->setMaximumHeight(50);
    layout->addWidget(m_optionEdit);

    // Transition text
    layout->addWidget(new QLabel(tr("Transition Text:"), this));
    m_transitionEdit = new QTextEdit(edge.transitionText, this);
    layout->addWidget(m_transitionEdit);

    // Condition formula
    layout->addWidget(new QLabel(tr("Condition Formula:"), this));
    m_conditionEdit = new QTextEdit(edge.condition, this);
    m_conditionEdit->setMaximumHeight(60);
    layout->addWidget(m_conditionEdit);

    m_conditionStatusLabel = new QLabel(this);
    layout->addWidget(m_conditionStatusLabel);

    // Buttons
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(m_buttons);

    connect(m_conditionEdit, &QTextEdit::textChanged, this, &EdgeDialog::updateConditionValidation);
    updateConditionValidation();
}

QString EdgeDialog::optionText() const {
    return m_optionEdit->toPlainText();
}

QString EdgeDialog::transitionText() const {
    return m_transitionEdit->toPlainText();
}

QString EdgeDialog::conditionText() const {
    return m_conditionEdit->toPlainText();
}

int EdgeDialog::edgeColor() const {
    return m_selectedColor;
}

void EdgeDialog::updateConditionValidation() {
    const QString condition = m_conditionEdit->toPlainText().trimmed();
    if (condition.isEmpty()) {
        m_conditionStatusLabel->setText(tr("Condition is empty: edge is always available"));
        m_conditionStatusLabel->setStyleSheet("color: #6a8f43;");
        m_buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
        return;
    }

    const auto parseResult = tge::formula::parse(condition.toStdString());
    if (parseResult.ast) {
        m_conditionStatusLabel->setText(tr("Formula parsed successfully"));
        m_conditionStatusLabel->setStyleSheet("color: #1d7d31;");
        m_buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
        return;
    }

    m_conditionStatusLabel->setText(tr("Parse error: %1").arg(QString::fromStdString(parseResult.error)));
    m_conditionStatusLabel->setStyleSheet("color: #b00020;");
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void EdgeDialog::onColorButtonClicked(int id) {
    if (id < tge::domain::LOCATION_COLOR_COUNT) {
        m_selectedColor = id;
    } else {
        m_selectedColor = tge::domain::LOCATION_COLOR_NONE;
    }
    updateColorSelection();
}

void EdgeDialog::updateColorSelection() {
    for (int i = 0; i < m_colorButtons.size(); ++i) {
        if ((m_selectedColor == tge::domain::LOCATION_COLOR_NONE && i == tge::domain::LOCATION_COLOR_COUNT) ||
            (m_selectedColor == i)) {
            m_colorButtons[i]->setStyleSheet(m_colorButtons[i]->styleSheet() + "; border: 3px solid #333;");
        } else {
            if (i < tge::domain::LOCATION_COLOR_COUNT) {
                m_colorButtons[i]->setStyleSheet(QString("background-color: %1; border: 1px solid #aaa;")
                                                 .arg(graphwidget_helpers::LOCATION_COLOR_PALETTE[i].name()));
            } else {
                m_colorButtons[i]->setStyleSheet("background: none; border: 1px dashed #aaa;");
            }
        }
    }
}
