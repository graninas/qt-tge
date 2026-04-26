#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QWidget>
#include <optional>
#include <variant>

#include "tge/domain.h"
#include "tge/player/runtime/engine.h"

class QLabel;
class QListWidget;
class QPushButton;
class QPlainTextEdit;
class QTableWidget;

class PlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerWidget(QWidget *parent = nullptr);

    void setGameDef(const tge::domain::GameDef *gameDef);
    bool startSession(QString *error = nullptr);
    void clearSession();

private slots:
    void onStartRestartClicked();
    void onConfirmClicked();
    void onOptionSelectionChanged();

private:
    struct OptionRow {
        int edgeId = 0;
        bool available = false;
    };

    void buildUi();
    void updateUi();
    void updateStepSections();
    void updateVariablesTable();
    void updateInfoDisplayTable();
    void updateConfirmButtonState();
    void updateOptionsForLocation(const tge::player::runtime::CurrentLocation &location);

    QString buildStepTitle() const;
    QString buildStepMessage() const;
    QStringList buildStepDebugMessages() const;
    QString buildPendingSummary(const tge::player::runtime::CurrentTransition &transition) const;

    const tge::domain::GameDef *m_gameDef = nullptr;
    std::optional<tge::player::runtime::Engine> m_engine;
    std::optional<tge::player::runtime::StepResult> m_currentStep;
    QVector<OptionRow> m_optionRows;
    int m_confirmationCount = 0;

    QLabel *m_stateLabel = nullptr;
    QLabel *m_confirmationsLabel = nullptr;
    QPlainTextEdit *m_messageEdit = nullptr;
    QListWidget *m_optionsList = nullptr;
    QPlainTextEdit *m_pendingEdit = nullptr;
    QPlainTextEdit *m_debugEdit = nullptr;
    QTableWidget *m_infoDisplayTable = nullptr;
    QTableWidget *m_variablesTable = nullptr;
    QPushButton *m_startRestartButton = nullptr;
    QPushButton *m_confirmButton = nullptr;
};

#endif // PLAYERWIDGET_H
