#include "TankTypeManager.h"

bool TankTypeManager::Initialize()
{
    Gameplay::BuiltInDefinitions();
    return true;
}

TankConfig TankTypeManager::GetConfig(TankType type1, TankType type2)
{
    return Gameplay::ResolveTankType(Gameplay::BuiltInDefinitions(), type1, type2);
}

void TankTypeManager::GetTankTypeColor(TankType type, float& r, float& g, float& b)
{
    const auto& types = Gameplay::BuiltInDefinitions().types;
    auto it = types.find(type);
    const auto& config = it == types.end() ? types.at(TankType::TYPE_GREY) : it->second;
    r = config.r;
    g = config.g;
    b = config.b;
}

void TankTypeManager::GetTankTypeColor(TankType type, float& r, float& g, float& b, float& a)
{
    GetTankTypeColor(type, r, g, b);
    a = 1.0f; // Default alpha for tank colors
}

Color TankTypeManager::GetTankTypeColor(TankType type)
{
    Color color;
    GetTankTypeColor(type, color.r, color.g, color.b, color.a);
    return color;
}
