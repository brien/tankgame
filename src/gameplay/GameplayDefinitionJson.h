#pragma once

#include "GameplayDefinitions.h"
#include <string>

namespace Gameplay {
// Strict version 1 snapshots. Throws std::invalid_argument with a field path.
GameplayDefinitions ParseDefinitionJson(const std::string& text);
std::string SerializeDefinitionJson(const GameplayDefinitions& definitions);
}
