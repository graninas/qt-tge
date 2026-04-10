#ifndef TGE_PLAYER_MODEL_H
#define TGE_PLAYER_MODEL_H

#include <QString>
#include <QVariant>
#include <QVector>
#include "../../tge/domain.h"

namespace tge {
namespace player {

struct VariableState {
    QString name;
    domain::VarType type;
    QVariant value;
};

} // namespace player
} // namespace tge

#endif // TGE_PLAYER_MODEL_H
