#include "edgedialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "tge/domain.h"

EdgeDialog::EdgeDialog(const tge::domain::EdgeDef& edge,
                       const tge::domain::LocationDef& fromLoc,
                       const tge::domain::LocationDef& toLoc,
                       QWidget* parent)
    : QDialog(parent)
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

    // Option text
    layout->addWidget(new QLabel(tr("Option Text:"), this));
    m_optionEdit = new QTextEdit(edge.optionText, this);
    m_optionEdit->setMaximumHeight(50);
    layout->addWidget(m_optionEdit);

    // Transition text
    layout->addWidget(new QLabel(tr("Transition Text:"), this));
    m_transitionEdit = new QTextEdit(edge.transitionText, this);
    layout->addWidget(m_transitionEdit);

    // Buttons
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString EdgeDialog::optionText() const {
    return m_optionEdit->toPlainText();
}

QString EdgeDialog::transitionText() const {
    return m_transitionEdit->toPlainText();
}
