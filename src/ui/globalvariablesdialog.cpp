#include "globalvariablesdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

using tge::domain::VarType;
using tge::domain::VariableDef;

GlobalVariablesDialog::GlobalVariablesDialog(QVector<VariableDef>& variables,
                                             const tge::editor::CapabilityMatrix& capabilities,
                                             QWidget* parent)
    : QDialog(parent)
    , m_variablesRef(variables)
    , m_workingVariables(variables)
    , m_capabilities(capabilities)
{
    setWindowTitle(tr("Global Variables"));
    resize(900, 520);

    QHBoxLayout* root = new QHBoxLayout(this);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel(tr("Variables:"), this));

    m_listWidget = new QListWidget(this);
    leftLayout->addWidget(m_listWidget, 1);

    QHBoxLayout* listButtons = new QHBoxLayout();
    m_addButton = new QPushButton(tr("Add"), this);
    m_removeButton = new QPushButton(tr("Remove"), this);
    listButtons->addWidget(m_addButton);
    listButtons->addWidget(m_removeButton);
    leftLayout->addLayout(listButtons);

    root->addLayout(leftLayout, 2);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel(tr("Selected Variable:"), this));

    QFormLayout* form = new QFormLayout();

    m_indexSpin = new QSpinBox(this);
    m_indexSpin->setMinimum(1);
    m_indexSpin->setMaximum(1000000000);
    m_indexSpin->setToolTip(tr("Unique numeric index; stored as P<index>."));
    form->addRow(tr("Index:"), m_indexSpin);

    m_nameEdit = new QLineEdit(this);
    form->addRow(tr("Name:"), m_nameEdit);

    m_descriptionEdit = new QTextEdit(this);
    m_descriptionEdit->setMinimumHeight(140);
    form->addRow(tr("Description:"), m_descriptionEdit);

    m_typeCombo = new QComboBox(this);
    m_typeCombo->addItem(tr("Integer"), static_cast<int>(VarType::Integer));
    form->addRow(tr("Type:"), m_typeCombo);

    m_defaultValueEdit = new QLineEdit(this);
    m_defaultValueEdit->setPlaceholderText(tr("0"));
    form->addRow(tr("Default Value:"), m_defaultValueEdit);

    rightLayout->addLayout(form, 1);

    m_statusLabel = new QLabel(this);
    rightLayout->addWidget(m_statusLabel);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    rightLayout->addWidget(m_buttonBox);

    root->addLayout(rightLayout, 3);

    connect(m_addButton, &QPushButton::clicked, this, &GlobalVariablesDialog::onAddVariable);
    connect(m_removeButton, &QPushButton::clicked, this, &GlobalVariablesDialog::onRemoveVariable);
    connect(m_listWidget, &QListWidget::currentRowChanged, this, &GlobalVariablesDialog::onSelectionChanged);

    connect(m_indexSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { onEditorChanged(); });
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString&) { onEditorChanged(); });
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &GlobalVariablesDialog::onEditorChanged);
    connect(m_typeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { onEditorChanged(); });
    connect(m_defaultValueEdit, &QLineEdit::textChanged, this, [this](const QString&) { onEditorChanged(); });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &GlobalVariablesDialog::onAccepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    rebuildList();
    if (!m_workingVariables.isEmpty()) {
        m_listWidget->setCurrentRow(0);
    } else {
        onSelectionChanged(-1);
    }
    validateAndShowStatus();
}

void GlobalVariablesDialog::rebuildList()
{
    m_listWidget->clear();
    for (int i = 0; i < m_workingVariables.size(); ++i) {
        const VariableDef& var = m_workingVariables[i];
        const QString shownName = var.name.trimmed().isEmpty() ? tr("(no name)") : var.name;
        m_listWidget->addItem(QString("%1 | %2").arg(var.index, shownName));
    }
}

void GlobalVariablesDialog::updateEditorEnabledState()
{
    const bool hasSelection = (m_currentRow >= 0 && m_currentRow < m_workingVariables.size());
    m_indexSpin->setEnabled(hasSelection && m_capabilities.allowGlobalVariableIndexEdit);
    m_nameEdit->setEnabled(hasSelection && m_capabilities.allowGlobalVariableNameEdit);
    m_descriptionEdit->setEnabled(hasSelection && m_capabilities.allowGlobalVariableDescriptionEdit);
    m_typeCombo->setEnabled(hasSelection && m_capabilities.allowGlobalVariableTypeEdit);
    m_defaultValueEdit->setEnabled(hasSelection && m_capabilities.allowGlobalVariableDefaultValueEdit);
    m_addButton->setEnabled(m_capabilities.allowGlobalVariableCreateDelete);
    m_removeButton->setEnabled(hasSelection && m_capabilities.allowGlobalVariableCreateDelete);
}

void GlobalVariablesDialog::saveEditorToCurrentRow()
{
    if (m_currentRow < 0 || m_currentRow >= m_workingVariables.size()) {
        return;
    }

    VariableDef& var = m_workingVariables[m_currentRow];
    if (m_capabilities.allowGlobalVariableIndexEdit) {
        var.index = indexToText(m_indexSpin->value());
    }
    if (m_capabilities.allowGlobalVariableNameEdit) {
        var.name = m_nameEdit->text();
    }
    if (m_capabilities.allowGlobalVariableDescriptionEdit) {
        var.description = m_descriptionEdit->toPlainText();
    }

    if (m_capabilities.allowGlobalVariableTypeEdit) {
        const auto typeValue = static_cast<VarType>(m_typeCombo->currentData().toInt());
        var.type = typeValue;
    }

    if (m_capabilities.allowGlobalVariableDefaultValueEdit) {
        var.defaultValue = m_defaultValueEdit->text().trimmed();
        if (var.defaultValue.isEmpty()) {
            var.defaultValue = "0";
        }
    }
}

void GlobalVariablesDialog::loadRowToEditor(int row)
{
    m_loadingEditor = true;

    if (row < 0 || row >= m_workingVariables.size()) {
        m_indexSpin->setValue(1);
        m_nameEdit->clear();
        m_descriptionEdit->clear();
        m_typeCombo->setCurrentIndex(0);
        m_defaultValueEdit->clear();
        m_loadingEditor = false;
        return;
    }

    const VariableDef& var = m_workingVariables[row];
    m_indexSpin->setValue(parseIndexToInt(var.index));
    m_nameEdit->setText(var.name);
    m_descriptionEdit->setPlainText(var.description);

    const int desiredType = static_cast<int>(var.type);
    int comboIndex = m_typeCombo->findData(desiredType);
    if (comboIndex < 0) {
        comboIndex = 0;
    }
    m_typeCombo->setCurrentIndex(comboIndex);

    m_defaultValueEdit->setText(var.defaultValue);
    m_loadingEditor = false;
}

void GlobalVariablesDialog::refreshRowCaption(int row)
{
    if (row < 0 || row >= m_workingVariables.size()) {
        return;
    }

    const VariableDef& var = m_workingVariables[row];
    const QString shownName = var.name.trimmed().isEmpty() ? tr("(no name)") : var.name;
    if (QListWidgetItem* item = m_listWidget->item(row)) {
        item->setText(QString("%1 | %2").arg(var.index, shownName));
    }
}

void GlobalVariablesDialog::validateAndShowStatus()
{
    QString error;
    const bool ok = validateAll(&error);
    if (ok) {
        m_statusLabel->setText(tr("All variables are valid."));
        m_statusLabel->setStyleSheet("color: #1d7d31;");
    } else {
        m_statusLabel->setText(error);
        m_statusLabel->setStyleSheet("color: #b00020;");
    }

    if (QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(ok);
    }
}

bool GlobalVariablesDialog::validateAll(QString* errorMessage) const
{
    QSet<int> seenIndexes;

    for (int i = 0; i < m_workingVariables.size(); ++i) {
        const VariableDef& var = m_workingVariables[i];

        const int indexValue = parseIndexToInt(var.index);
        if (indexValue <= 0) {
            if (errorMessage) {
                *errorMessage = tr("Variable %1 has invalid index: %2").arg(i + 1).arg(var.index);
            }
            return false;
        }

        if (seenIndexes.contains(indexValue)) {
            if (errorMessage) {
                *errorMessage = tr("Duplicate index P%1. Indexes must be unique.").arg(indexValue);
            }
            return false;
        }
        seenIndexes.insert(indexValue);

        bool defaultOk = false;
        var.defaultValue.toInt(&defaultOk);
        if (!defaultOk) {
            if (errorMessage) {
                *errorMessage = tr("Default value for %1 must be an integer.").arg(var.index);
            }
            return false;
        }
    }

    return true;
}

int GlobalVariablesDialog::firstFreeIndex() const
{
    QSet<int> used;
    for (const VariableDef& var : m_workingVariables) {
        const int idx = parseIndexToInt(var.index);
        if (idx > 0) {
            used.insert(idx);
        }
    }

    int candidate = 1;
    while (used.contains(candidate)) {
        ++candidate;
    }
    return candidate;
}

int GlobalVariablesDialog::parseIndexToInt(const QString& indexText)
{
    const QString trimmed = indexText.trimmed();

    bool ok = false;
    int value = 0;
    if (trimmed.startsWith('P', Qt::CaseInsensitive)) {
        value = trimmed.mid(1).toInt(&ok);
    } else {
        value = trimmed.toInt(&ok);
    }

    if (!ok || value <= 0) {
        return 1;
    }
    return value;
}

QString GlobalVariablesDialog::indexToText(int indexValue)
{
    return QString("P%1").arg(indexValue);
}

void GlobalVariablesDialog::onAddVariable()
{
    if (!m_capabilities.allowGlobalVariableCreateDelete) {
        return;
    }

    saveEditorToCurrentRow();

    VariableDef var;
    const int nextIndex = firstFreeIndex();
    var.index = indexToText(nextIndex);
    var.name = tr("Variable %1").arg(nextIndex);
    var.description = QString();
    var.type = VarType::Integer;
    var.defaultValue = "0";

    m_workingVariables.append(var);
    rebuildList();
    m_listWidget->setCurrentRow(m_workingVariables.size() - 1);
    validateAndShowStatus();
}

void GlobalVariablesDialog::onRemoveVariable()
{
    if (!m_capabilities.allowGlobalVariableCreateDelete) {
        return;
    }

    if (m_currentRow < 0 || m_currentRow >= m_workingVariables.size()) {
        return;
    }

    m_workingVariables.removeAt(m_currentRow);
    rebuildList();

    if (m_workingVariables.isEmpty()) {
        m_listWidget->setCurrentRow(-1);
        onSelectionChanged(-1);
    } else {
        const int newRow = qMin(m_currentRow, m_workingVariables.size() - 1);
        m_listWidget->setCurrentRow(newRow);
    }

    validateAndShowStatus();
}

void GlobalVariablesDialog::onSelectionChanged(int newRow)
{
    if (!m_loadingEditor) {
        saveEditorToCurrentRow();
    }

    m_currentRow = newRow;
    loadRowToEditor(newRow);
    updateEditorEnabledState();
    validateAndShowStatus();
}

void GlobalVariablesDialog::onEditorChanged()
{
    if (m_loadingEditor) {
        return;
    }

    saveEditorToCurrentRow();
    refreshRowCaption(m_currentRow);
    validateAndShowStatus();
}

void GlobalVariablesDialog::onAccepted()
{
    saveEditorToCurrentRow();

    QString error;
    if (!validateAll(&error)) {
        QMessageBox::warning(this, tr("Invalid Variables"), error);
        return;
    }

    m_variablesRef = m_workingVariables;
    accept();
}
