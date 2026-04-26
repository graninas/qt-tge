#ifndef TGE_DOMAIN_H
#define TGE_DOMAIN_H

#include <QString>
#include <QVector>
#include <QMap>

namespace tge {
namespace domain {

constexpr int LOCATION_COLOR_NONE = -1;
constexpr int LOCATION_COLOR_COUNT = 15;

// Supported variable types
enum class VarType { Integer };

// Variable definition
struct VariableDef {
    QString index; // Identifier: P1, P2, etc.
    QString name;
    QString description;
    VarType type;
    QString defaultValue; // Default initial value, stored as string and interpreted according to type
};

// Edge variable setting definition
struct EdgeVariableSettingDef {
    int variableIndex; // Identifier of the parameter to set, e.g. (P)1
    QString edgeVariableCondition; // Formula that must evaluate to true for this edge to be available (plus global edge condition), can be empty for always available
    QString newValueFormula; // Formula to compute the new value of the parameter when this edge is taken
};

struct EdgeInfoDisplayItemSettingDef {
    int itemIndex; // Identifier (id) of the info display item to set
    bool changePriority; // Whether to change the priority of the item
    int newPriority; // New priority value, used if changePriority is true
    bool changeVisibility; // Whether to change the visibility of the item
    bool newVisibility; // New visibility value, used if changeVisibility is true
    bool changeShowValue; // Whether to change the show value setting of the item
    bool newShowValue; // New show value setting, used if changeShowValue is true
};

// Edge definition
struct EdgeDef {
    int id;
    int fromLocation; // index or id of source location
    int toLocation;   // index or id of destination location

    QString optionText;
    QString transitionText;
    QString condition; // Formula that must evaluate to true for this edge to be available (plus edge variable conditions), can be empty for always available
    QVector<EdgeVariableSettingDef> variableSettings; // Variable settings applied when this edge is taken
    QVector<EdgeInfoDisplayItemSettingDef> infoDisplayItemSettings; // Info display item settings applied when this edge is taken
    int color = LOCATION_COLOR_NONE; // Palette index, -1 means default edge color
    int priority = 0; // Display/selection priority; lower value = shown first
};

// Location definition
enum class LocationType { Start, Regular, Service, Finish };

struct LocationDef {
    int id;
    LocationType type;
    QString description;
    QVector<int> outgoingEdges;
    QVector<int> incomingEdges;
    // UX domain model fields (editor only)
    int coordX, coordY;
    QString label;
    int color = LOCATION_COLOR_NONE; // Palette index, -1 means no color
};

enum class InfoDisplayItemMode { Actual, Debug };

// Game info display model (editor only)
struct InfoDisplayItemDef {
    int id = 0;
    QString label;
    QString valueFormula; // Formula to compute the value to display
    InfoDisplayItemMode mode = InfoDisplayItemMode::Actual;
    int priority = 0; // Lower value means higher order in display
    bool isVisible = false; // false = hidden, true = shown
    bool showFormulaValue = true; // true = show computed formula value, false = hide it
};

// Game definition
struct GameDef {
    QString name;
    QString description;
    QMap<int, LocationDef> locations; // id -> location
    QMap<int, EdgeDef> edges; // id -> edge
    QVector<VariableDef> globalVariables;
    QVector<InfoDisplayItemDef> infoDisplayItems;
};

} // namespace domain
} // namespace tge

#endif // TGE_DOMAIN_H
