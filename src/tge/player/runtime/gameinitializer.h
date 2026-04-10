#ifndef TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
#define TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H

#include "../../domain.h"
#include "../types.h"

namespace tge {
namespace player {
namespace runtime {

class GameInitializer {
public:
    GameInitializer(const domain::GameDef& /*gameDef*/) {}
    GameState initialize() { return GameState{}; }
};

} // namespace runtime
} // namespace player
} // namespace tge

#endif // TGE_PLAYER_RUNTIME_GAMEINITIALIZER_H
