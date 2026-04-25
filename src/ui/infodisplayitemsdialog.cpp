#include "infodisplayitemsdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include "tge/formula/parser.h"

using tge::domain::InfoDisplayItemDef;
using tge::domain::InfoDisplayItemMode;

InfoDisplayItemsDialog::InfoDisplayItemsDialog(QVector<InfoDisplayItemDef>& items, QWidget* parent)
    : QDialog(parent)
    , m_itemsRef(items)
    , m_workingItems(items)
{
    setWindowTitle(tr("Info Display Items"));
    resize(900, 520);

    QHBoxLayout* root = new QHBoxLayout(this);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel(tr("Items:"), this));

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
    rightLayout->addWidget(new QLabel(tr("Selected Item:"), this));

    QFormLayout* form = new QFormLayout();

    m_labelEdit = new QLineEdit(this);
    form->addRow(tr("Label:"), m_labelEdit);

    m_formulaEdit = new QTextEdit(this);
    m_formulaEdit->setMinimumHeight(140);
    form->addRow(tr("Value Formula:"), m_formulaEdit);

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(tr("Actual gameplay"), static_cast<int>(InfoDisplayItemMode::Actual));
    m_modeCombo->addItem(tr("Debug"), static_cast<int>(InfoDisplayItemMode::Debug));
    form->addRow(tr("Mode:"), m_modeCombo);

    m_prioritySpin = new QSpinBox(this);
    m_prioritySpin->setMinimum(-1000000000);
    m_prioritySpin->setMaximum(1000000000);
    form->addRow(tr("Priority:"), m_prioritySpin);

    QWidget* visibilityWidget = new QWidget(this);
    QHBoxLayout* visibilityLayout = new QHBoxLayout(visibilityWidget);
    visibilityLayout->setContentsMargins(0, 0, 0, 0);
    m_visibilityGroup = new QButtonGroup(this);
    m_hiddenRadio = new QRadioButton(tr("Hidden"), visibilityWidget);
    m_shownRadio = new QRadioButton(tr("Shown"), visibilityWidget);
    m_visibilityGroup->addButton(m_hiddenRadio, 0);
    m_visibilityGroup->addButton(m_shownRadio, 1);
    visibilityLayout->addWidget(m_hiddenRadio);
    visibilityLayout->addWidget(m_shownRadio);
    form->addRow(tr("Display item:"), visibilityWidget);

    QWidget* showValueWidget = new QWidget(this);
    QHBoxLayout* showValueLayout = new QHBoxLayout(showValueWidget);
    showValueLayout->setContentsMargins(0, 0, 0, 0);
    m_showValueGroup = new QButtonGroup(this);
    m_showValueYesRadio = new QRadioButton(tr("Yes"), showValueWidget);
    m_showValueNoRadio = new QRadioButton(tr("No"), showValueWidget);
    m_showValueGroup->addButton(m_showValueYesRadio, 1);
    m_showValueGroup->addButton(m_showValueNoRadio, 0);
    showValueLayout->addWidget(m_showValueYesRadio);
    showValueLayout->addWidget(m_showValueNoRadio);
    form->addRow(tr("Show formula value:"), showValueWidget);

    rightLayout->addLayout(form, 1);

    m_formulaStatusLabel = new QLabel(this);
    rightLayout->addWidget(m_formulaStatusLabel);

    m_statusLabel = new QLabel(this);
    rightLayout->addWidget(m_statusLabel);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    rightLayout->addWidget(m_buttonBox);

    root->addLayout(rightLayout, 3);

    connect(m_addButton, &QPushButton::clicked, this, &InfoDisplayItemsDialog::onAddItem);
    connect(m_removeButton, &QPushButton::clicked, this, &InfoDisplayItemsDialog::onRemoveItem);
    connect(m_listWidget, &QListWidget::currentRowChanged, this, &InfoDisplayItemsDialog::onSelectionChanged);

    connect(m_labelEdit, &QLineEdit::textChanged, this, [this](const QString&) { onEditorChanged(); });
    connect(m_formulaEdit, &QTextEdit::textChanged, this, &InfoDisplayItemsDialog::onEditorChanged);
    connect(m_modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) { onEditorChanged(); });
    connect(m_prioritySpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int) { onEditorChanged(); });
    connect(m_hiddenRadio, &QRadioButton::toggled, this, [this](bool) { onEditorChanged(); });
    connect(m_shownRadio, &QRadioButton::toggled, this, [this](bool) { onEditorChanged(); });
    connect(m_showValueYesRadio, &QRadioButton::toggled, this, [this](bool) { onEditorChanged(); });
    connect(m_showValueNoRadio, &QRadioButton::toggled, this, [this](bool) { onEditorChanged(); });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &InfoDisplayItemsDialog::onAccepted);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    rebuildList();
    if (!m_workingItems.isEmpty()) {
        m_listWidget->setCurrentRow(0);
    } else {
        onSelectionChanged(-1);
    }
    validateAndShowStatus();
}

void InfoDisplayItemsDialog::rebuildList()
{
    m_listWidget->clear();
    for (int i = 0; i < m_workingItems.size(); ++i) {
        const InfoDisplayItemDef& item = m_workingItems[i];
        const QString shownLabel = item.label.trimmed().isEmpty() ? tr("(no label)") : item.label;
        const QString modeSuffix = (item.mode == InfoDisplayItemMode::Debug) ? tr(" [Debug]") : tr(" [Actual]");
        const QString visibilitySuffix = item.isVisible ? tr(" [Shown]") : tr(" [Hidden]");
        m_listWidget->addItem(QString("%1 (p=%2)%3%4")
                                  .arg(shownLabel)
                                  .arg(item.priority)
                                  .arg(modeSuffix)
                                  .arg(visibilitySuffix));
    }
}

void InfoDisplayItemsDialog::updateEditorEnabledState()
{
    const bool hasSelection = (m_currentRow >= 0 && m_currentRow < m_workingItems.size());
    m_labelEdit->setEnabled(hasSelection);
    m_formulaEdit->setEnabled(hasSelection);
    m_modeCombo->setEnabled(hasSelection);
    m_prioritySpin->setEnabled(hasSelection);
    m_hiddenRadio->setEnabled(hasSelection);
    m_shownRadio->setEnabled(hasSelection);
    m_showValueYesRadio->setEnabled(hasSelection);
    m_showValueNoRadio->setEnabled(hasSelection);
    m_removeButton->setEnabled(hasSelection);
}

void InfoDisplayItemsDialog::saveEditorToCurrentRow()
{
    if (m_currentRow < 0 || m_currentRow >= m_workingItems.size()) {
        return;
    }

    InfoDisplayItemDef& item = m_workingItems[m_currentRow];
    item.label = m_labelEdit->text();
    item.valueFormula = m_formulaEdit->toPlainText().trimmed();
    item.mode = static_cast<InfoDisplayItemMode>(m_modeCombo->currentData().toInt());
    item.priority = m_prioritySpin->value();
    item.isVisible = m_shownRadio->isChecked();
    item.showFormulaValue = m_showValueYesRadio->isChecked();
}

void InfoDisplayItemsDialog::loadRowToEditor(int row)
{
    m_loadingEditor = true;

    if (row < 0 || row >= m_workingItems.size()) {
        m_labelEdit->clear();
        m_formulaEdit->clear();
        m_modeCombo->setCurrentIndex(0);
        m_prioritySpin->setValue(0);
        m_hiddenRadio->setChecked(true);
        m_showValueYesRadio->setChecked(true);
        m_loadingEditor = false;
        return;
    }

    const InfoDisplayItemDef& item = m_workingItems[row];
    m_labelEdit->setText(item.label);
    m_formulaEdit->setPlainText(item.valueFormula);

    const int desiredMode = static_cast<int>(item.mode);
    int comboIndex = m_modeCombo->findData(desiredMode);
    if (comboIndex < 0) {
        comboIndex = 0;
    }
    m_modeCombo->setCurrentIndex(comboIndex);
    m_prioritySpin->setValue(item.priority);
    if (item.isVisible) {
        m_shownRadio->setChecked(true);
    } else {
        m_hiddenRadio->setChecked(true);
    }
    if (item.showFormulaValue) {
        m_showValueYesRadio->setChecked(true);
    } else {
        m_showValueNoRadio->setChecked(true);
    }

    m_loadingEditor = false;
}

void InfoDisplayItemsDialog::refreshRowCaption(int row)
{
    if (row < 0 || row >= m_workingItems.size()) {
        return;
    }

    const InfoDisplayItemDef& item = m_workingItems[row];
    const QString shownLabel = item.label.trimmed().isEmpty() ? tr("(no label)") : item.label;
    const QString modeSuffix = (item.mode == InfoDisplayItemMode::Debug) ? tr(" [Debug]") : tr(" [Actual]");
    const QString visibilitySuffix = item.isVisible ? tr(" [Shown]") : tr(" [Hidden]");
    if (QListWidgetItem* listItem = m_listWidget->item(row)) {
        listItem->setText(QString("%1 (p=%2)%3%4")
                              .arg(shownLabel)
                              .arg(item.priority)
                              .arg(modeSuffix)
                              .arg(visibilitySuffix));
    }
}

void InfoDisplayItemsDialog::validateAndShowStatus()
{
    if (m_currentRow >= 0 && m_currentRow < m_workingItems.size()) {
        const QString formula = m_formulaEdit->toPlainText().trimmed();
        if (formula.isEmpty()) {
            m_formulaStatusLabel->setText(tr("Value formula is empty"));
            m_formulaStatusLabel->setStyleSheet("color: #b00020;");
        } else {
            const auto parseResult = tge::formula::parse(formula.toStdString());
            if (parseResult.ast) {
                m_formulaStatusLabel->setText(tr("Formula parsed successfully"));
                m_formulaStatusLabel->setStyleSheet("color: #1d7d31;");
            } else {
                m_formulaStatusLabel->setText(tr("Parse error: %1").arg(QString::fromStdString(parseResult.error)));
                m_formulaStatusLabel->setStyleSheet("color: #b00020;");
            }
        }
    } else {
        m_formulaStatusLabel->setText(tr("Select an item to edit"));
        m_formulaStatusLabel->setStyleSheet("color: #6a8f43;");
    }

    QString error;
    const bool ok = validateAll(&error);
    if (ok) {
        m_statusLabel->setText(tr("All info display items are valid."));
        m_statusLabel->setStyleSheet("color: #1d7d31;");
    } else {
        m_statusLabel->setText(error);
        m_statusLabel->setStyleSheet("color: #b00020;");
    }

    if (QPushButton* okButton = m_buttonBox->button(QDialogButtonBox::Ok)) {
        okButton->setEnabled(ok);
    }
}

bool InfoDisplayItemsDialog::validateAll(QString* errorMessage) const
{
    for (int i = 0; i < m_workingItems.size(); ++i) {
        const InfoDisplayItemDef& item = m_workingItems[i];
        if (item.label.trimmed().isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Item %1 has empty label").arg(i + 1);
            }
            return false;
        }

        const QString formula = item.valueFormula.trimmed();
        if (formula.isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("Item %1 (%2) has empty value formula").arg(i + 1).arg(item.label);
            }
            return false;
        }

        const auto parseResult = tge::formula::parse(formula.toStdString());
        if (!parseResult.ast) {
            if (errorMessage) {
                *errorMessage = tr("Item %1 (%2): parse error: %3")
                                    .arg(i + 1)
                                    .arg(item.label)
                                    .arg(QString::fromStdString(parseResult.error));
            }
            return false;
        }
    }

    return true;
}

void InfoDisplayItemsDialog::onAddItem()
{
    saveEditorToCurrentRow();

    InfoDisplayItemDef item;
    item.label = tr("Item %1").arg(m_workingItems.size() + 1);
    item.valueFormula = "0";
    item.mode = InfoDisplayItemMode::Actual;
    item.priority = m_workingItems.size();
    item.isVisible = false;
    item.showFormulaValue = true;

    m_workingItems.append(item);
    rebuildList();
    m_listWidget->setCurrentRow(m_workingItems.size() - 1);
    validateAndShowStatus();
}

void InfoDisplayItemsDialog::onRemoveItem()
{
    if (m_currentRow < 0 || m_currentRow >= m_workingItems.size()) {
        return;
    }

    m_workingItems.removeAt(m_currentRow);
    rebuildList();

    if (m_workingItems.isEmpty()) {
        m_listWidget->setCurrentRow(-1);
        onSelectionChanged(-1);
    } else {
        const int newRow = qMin(m_currentRow, m_workingItems.size() - 1);
        m_listWidget->setCurrentRow(newRow);
    }

    validateAndShowStatus();
}

void InfoDisplayItemsDialog::onSelectionChanged(int newRow)
{
    if (!m_loadingEditor) {
        saveEditorToCurrentRow();
    }

    m_currentRow = newRow;
    loadRowToEditor(newRow);
    updateEditorEnabledState();
    validateAndShowStatus();
}

void InfoDisplayItemsDialog::onEditorChanged()
{
    if (m_loadingEditor) {
        return;
    }

    saveEditorToCurrentRow();
    refreshRowCaption(m_currentRow);
    validateAndShowStatus();
}

void InfoDisplayItemsDialog::onAccepted()
{
    saveEditorToCurrentRow();

    QString error;
    if (!validateAll(&error)) {
        QMessageBox::warning(this, tr("Invalid Info Display Items"), error);
        return;
    }

    m_itemsRef = m_workingItems;
    accept();
}
