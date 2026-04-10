#ifndef TGE_DOMAIN_H
#define TGE_DOMAIN_H

#include <QString>
#include <QVector>
#include <QMap>

namespace tge {
namespace domain {

// Supported variable types
enum class VarType { String, Integer, Float, Char, Bool };

// Variable definition (static)
struct VariableDef {
    QString name;
    VarType type;
    QString defaultValue; // Default initial value, stored as string and interpreted according to type
    QString updateRule; // To be defined later
};

// Selector type
enum class SelectorType { Blank, Random, Formula };

// Selector definition
struct SelectorDef {
    SelectorType type;
    QString formula; // Used if type == Formula
};

// Text description pack
struct DescriptionPack {
    QVector<QString> descriptions;
    SelectorDef selector;
};

// Edge definition
struct EdgeDef {
    int id;
    int fromLocation; // index or id of source location
    int toLocation;   // index or id of destination location
    QString optionText;
    QString transitionText;
    // Edge type can be added here if needed
};

// Location definition
enum class LocationType { Start, Regular, Service, Finish };

struct LocationDef {
    int id;
    LocationType type;
    DescriptionPack descriptionPack;
    QVector<VariableDef> localVariables;
    QVector<EdgeDef> outgoingEdges;
    // UX domain model fields (editor only)
    int coordX, coordY;
    QString label;
    QString color;
};

// Game definition
struct GameDef {
    QString name;
    QString description;
    QVector<LocationDef> locations;
    QVector<VariableDef> globalVariables;
};

} // namespace domain
} // namespace tge

#endif // TGE_DOMAIN_H
