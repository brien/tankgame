#include "GameplayDefinitionJson.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
#include <type_traits>

namespace Gameplay {
namespace {
using Json = nlohmann::json;
constexpr std::size_t MaxBytes = 1024 * 1024;
const char* TypeNames[] = {"grey", "red", "blue", "yellow", "purple"};
const TankType Types[] = {TankType::TYPE_GREY, TankType::TYPE_RED, TankType::TYPE_BLUE,
    TankType::TYPE_YELLOW, TankType::TYPE_PURPLE};
const char* DifficultyNames[] = {"easy", "normal", "hard"};

[[noreturn]] void Fail(const std::string& path, const std::string& reason) {
    throw std::invalid_argument(path + ": " + reason);
}
std::string TypeName(TankType type) {
    for (int i = 0; i < 5; ++i) if (Types[i] == type) return TypeNames[i];
    Fail("type", "unknown tank type");
}
TankType ReadType(const Json& j, const std::string& path) {
    if (!j.is_string()) Fail(path, "expected a tank type name");
    for (int i = 0; i < 5; ++i) if (j == TypeNames[i]) return Types[i];
    Fail(path, "unknown tank type name");
}
void Keys(const Json& j, std::initializer_list<const char*> keys, const std::string& path) {
    if (!j.is_object()) Fail(path, "expected an object");
    std::set<std::string> allowed;
    for (auto key : keys) {
        allowed.insert(key);
        if (!j.count(key)) Fail(path + "." + key, "required field missing");
    }
    for (auto it = j.begin(); it != j.end(); ++it)
        if (!allowed.count(it.key())) Fail(path + "." + it.key(), "unknown field");
}

template<class T, class Enable = void> struct Codec;
template<class T> Json Encode(const T& value) { return Codec<T>::Write(value); }
template<class T> void Decode(const Json& j, T& value, const std::string& path) {
    Codec<T>::Read(j, value, path);
}

template<class T> struct Codec<T, typename std::enable_if<std::is_integral<T>::value>::type> {
    static Json Write(T value) { return value; }
    static void Read(const Json& j, T& value, const std::string& path) {
        if (!j.is_number_integer()) Fail(path, "expected an integer");
        if (j.is_number_unsigned()) {
            if (j.get<std::uint64_t>() > static_cast<std::uint64_t>(std::numeric_limits<T>::max()))
                Fail(path, "integer out of range");
        } else if (j.get<std::int64_t>() < std::numeric_limits<T>::min() ||
                   j.get<std::int64_t>() > std::numeric_limits<T>::max()) {
            Fail(path, "integer out of range");
        }
        value = j.get<T>();
    }
};
template<> struct Codec<float> {
    static Json Write(float value) { return value; }
    static void Read(const Json& j, float& value, const std::string& path) {
        if (!j.is_number()) Fail(path, "expected a number");
        const double number = j.get<double>();
        if (!std::isfinite(number) || std::abs(number) > std::numeric_limits<float>::max() ||
            (number != 0 && static_cast<float>(number) == 0)) Fail(path, "float out of range");
        value = static_cast<float>(number);
    }
};
template<> struct Codec<TankType> {
    static Json Write(TankType value) { return TypeName(value); }
    static void Read(const Json& j, TankType& value, const std::string& path) { value = ReadType(j, path); }
};
template<> struct Codec<std::pair<TankType, TankType>> {
    static Json Write(const std::pair<TankType, TankType>& value) {
        return {{"primary", TypeName(value.first)}, {"secondary", TypeName(value.second)}};
    }
    static void Read(const Json& j, std::pair<TankType, TankType>& value, const std::string& path) {
        Keys(j, {"primary", "secondary"}, path);
        value = {ReadType(j.at("primary"), path + ".primary"), ReadType(j.at("secondary"), path + ".secondary")};
    }
};
template<class T> struct Codec<std::vector<T>> {
    static Json Write(const std::vector<T>& value) {
        if (value.size() > 1024) Fail("array", "array exceeds 1024 entries");
        Json result = Json::array();
        for (const auto& item : value) result.push_back(Encode(item));
        return result;
    }
    static void Read(const Json& j, std::vector<T>& value, const std::string& path) {
        if (!j.is_array()) Fail(path, "expected an array");
        if (j.size() > 1024) Fail(path, "array exceeds 1024 entries");
        value.clear();
        for (std::size_t i = 0; i < j.size(); ++i) {
            T item{};
            Decode(j[i], item, path + "[" + std::to_string(i) + "]");
            value.push_back(item);
        }
    }
};
struct Writer {
    Json result = Json::object();
    template<class T> void Field(const char* name, T& value) { result[name] = Encode(value); }
};
struct Reader {
    const Json& source;
    std::string path;
    std::set<std::string> fields;
    Reader(const Json& source, std::string path) : source(source), path(std::move(path)) {
        if (!source.is_object()) Fail(this->path, "expected an object");
    }
    template<class T> void Field(const char* name, T& value) {
        fields.insert(name);
        if (!source.count(name)) Fail(path + "." + name, "required field missing");
        Decode(source.at(name), value, path + "." + name);
    }
    void Finish() {
        for (auto it = source.begin(); it != source.end(); ++it)
            if (!fields.count(it.key())) Fail(path + "." + it.key(), "unknown field");
    }
};
template<class A> void Visit(TankTypeConfig& v, A& a) {
    a.Field("fireIntervalSeconds", v.fireIntervalSeconds);
    a.Field("attack", v.attack);
    a.Field("maxEnergy", v.maxEnergy);
    a.Field("energyRegen", v.energyRegen);
    a.Field("moveCost", v.moveCost);
    a.Field("fireCost", v.fireCost);
    a.Field("specialChargeCost", v.specialChargeCost);
    a.Field("bounces", v.bounces);
    a.Field("r", v.r);
    a.Field("g", v.g);
    a.Field("b", v.b);
}
template<class A> void Visit(TankDefaults& v, A& a) {
    a.Field("maxHealth", v.maxHealth);
    a.Field("healthRegenPerSecond", v.healthRegenPerSecond);
    a.Field("moveSpeed", v.moveSpeed);
    a.Field("rotationDegreesPerSecond", v.rotationDegreesPerSecond);
    a.Field("collisionRadius", v.collisionRadius);
    a.Field("jumpEnergyPerSecond", v.jumpEnergyPerSecond);
    a.Field("jumpLaunchSpeed", v.jumpLaunchSpeed);
    a.Field("jumpThrust", v.jumpThrust);
    a.Field("gravity", v.gravity);
    a.Field("maxRiseSpeed", v.maxRiseSpeed);
    a.Field("maxFallSpeed", v.maxFallSpeed);
    a.Field("turboEnergyPerSecond", v.turboEnergyPerSecond);
    a.Field("turretInputScale", v.turretInputScale);
    a.Field("turretMaxStepDegrees", v.turretMaxStepDegrees);
    a.Field("maxForwardInput", v.maxForwardInput);
}
template<class A> void Visit(ProjectileRules& v, A& a) {
    a.Field("speed", v.speed);
    a.Field("lifetimeSeconds", v.lifetimeSeconds);
    a.Field("collisionRadius", v.collisionRadius);
    a.Field("registrationRadius", v.registrationRadius);
    a.Field("selfHitDelaySeconds", v.selfHitDelaySeconds);
    a.Field("purplePrimaryTurnScale", v.purplePrimaryTurnScale);
    a.Field("purpleSecondaryTurnScale", v.purpleSecondaryTurnScale);
    a.Field("purpleSecondaryPowerScale", v.purpleSecondaryPowerScale);
    a.Field("purpleSecondaryChargeDivisor", v.purpleSecondaryChargeDivisor);
    a.Field("purpleMinPower", v.purpleMinPower);
    a.Field("purpleMaxPower", v.purpleMaxPower);
    a.Field("purpleSpecialTurnAcceleration", v.purpleSpecialTurnAcceleration);
    a.Field("firstBouncePowerMultiplier", v.firstBouncePowerMultiplier);
    a.Field("yellowFirstBouncePowerMultiplier", v.yellowFirstBouncePowerMultiplier);
    a.Field("yellowRedChildCount", v.yellowRedChildCount);
    a.Field("yellowBlueChildCount", v.yellowBlueChildCount);
    a.Field("yellowBlueChildSpacing", v.yellowBlueChildSpacing);
    a.Field("childBounces", v.childBounces);
    a.Field("purpleChildBounces", v.purpleChildBounces);
    a.Field("purpleChildPowerMultiplier", v.purpleChildPowerMultiplier);
    a.Field("purpleChildTurnSpeed", v.purpleChildTurnSpeed);
}
template<class A> void Visit(PlayerRules& v, A& a) {
    a.Field("startingPrimaryType", v.startingPrimaryType);
    a.Field("startingSecondaryType", v.startingSecondaryType);
    a.Field("spawnHeight", v.spawnHeight);
    a.Field("respawnDelaySeconds", v.respawnDelaySeconds);
    a.Field("versusRespawnDelaySeconds", v.versusRespawnDelaySeconds);
    a.Field("comboDecayPerSecond", v.comboDecayPerSecond);
    a.Field("comboChargeFraction", v.comboChargeFraction);
    a.Field("specialChargeMax", v.specialChargeMax);
    a.Field("matchingPickupEnergyFraction", v.matchingPickupEnergyFraction);
    a.Field("otherPickupEnergyFraction", v.otherPickupEnergyFraction);
    a.Field("pickupScore", v.pickupScore);
}
template<class A> void Visit(CombatRules& v, A& a) {
    a.Field("healthReturnDivisor", v.healthReturnDivisor);
    a.Field("energyReturnDivisor", v.energyReturnDivisor);
    a.Field("friendlyResourceThresholdMultiplier", v.friendlyResourceThresholdMultiplier);
    a.Field("blueSpecialHitPowerBonus", v.blueSpecialHitPowerBonus);
    a.Field("hitComboDivisor", v.hitComboDivisor);
    a.Field("streakThreshold", v.streakThreshold);
    a.Field("streakComboDivisor", v.streakComboDivisor);
    a.Field("distanceThreshold", v.distanceThreshold);
    a.Field("distanceComboDivisor", v.distanceComboDivisor);
    a.Field("bounceBonusMaxAgeSeconds", v.bounceBonusMaxAgeSeconds);
    a.Field("bounceBonusBase", v.bounceBonusBase);
    a.Field("bounceAgeDivisor", v.bounceAgeDivisor);
    a.Field("killBonus", v.killBonus);
    a.Field("killComboDivisor", v.killComboDivisor);
}
template<class A> void Visit(InputRules& v, A& a) {
    a.Field("mouseTurretScale", v.mouseTurretScale);
    a.Field("keyboardTurretRate", v.keyboardTurretRate);
    a.Field("keyboardTurboTurretRate", v.keyboardTurboTurretRate);
    a.Field("browserPadActivationDeadzone", v.browserPadActivationDeadzone);
}
template<class A> void Visit(JoystickRules& v, A& a) {
    a.Field("forwardDeadzone", v.forwardDeadzone);
    a.Field("reverseDeadzone", v.reverseDeadzone);
    a.Field("bodyDeadzone", v.bodyDeadzone);
    a.Field("turretDeadzone", v.turretDeadzone);
    a.Field("alternateTurretDeadzone", v.alternateTurretDeadzone);
    a.Field("forwardDivisor", v.forwardDivisor);
    a.Field("bodyDivisor", v.bodyDivisor);
    a.Field("turretDivisor", v.turretDivisor);
    a.Field("chargeDivisor", v.chargeDivisor);
    a.Field("specialChargeDivisor", v.specialChargeDivisor);
    a.Field("fireAxisThreshold", v.fireAxisThreshold);
    a.Field("jumpAxisThreshold", v.jumpAxisThreshold);
}
template<class A> void Visit(SpecialRules& v, A& a) {
    a.Field("muzzleHeight", v.muzzleHeight);
    a.Field("blueCenterHeight", v.blueCenterHeight);
    a.Field("blueSideOffset", v.blueSideOffset);
    a.Field("redArcCount", v.redArcCount);
    a.Field("redArcStepDegrees", v.redArcStepDegrees);
    a.Field("redArcStartDegrees", v.redArcStartDegrees);
    a.Field("redArcBounces", v.redArcBounces);
    a.Field("yellowBounces", v.yellowBounces);
    a.Field("redForwardAnglesDegrees", v.redForwardAnglesDegrees);
    a.Field("purpleAnglesDegrees", v.purpleAnglesDegrees);
}
template<class A> void Visit(LandingRules& v, A& a) {
    a.Field("rewardEnergyFraction", v.rewardEnergyFraction);
    a.Field("rewardThresholdFraction", v.rewardThresholdFraction);
}
template<class A> void Visit(EnemyBehaviorRules& v, A& a) {
    a.Field("activationDistance", v.activationDistance);
    a.Field("activationDistancePerLevel", v.activationDistancePerLevel);
    a.Field("huntEnergyFraction", v.huntEnergyFraction);
    a.Field("fearUntilLevelOffset", v.fearUntilLevelOffset);
    a.Field("attackerCountBase", v.attackerCountBase);
    a.Field("wanderProbeAngleDegrees", v.wanderProbeAngleDegrees);
    a.Field("wanderProbeFrames", v.wanderProbeFrames);
    a.Field("wanderJumpHeight", v.wanderJumpHeight);
    a.Field("wanderMaxDrop", v.wanderMaxDrop);
    a.Field("wanderTurnDegrees", v.wanderTurnDegrees);
    a.Field("jumpEnergyFraction", v.jumpEnergyFraction);
    a.Field("aimToleranceDegrees", v.aimToleranceDegrees);
    a.Field("fearProbeAngleDegrees", v.fearProbeAngleDegrees);
    a.Field("fearProbeFrames", v.fearProbeFrames);
    a.Field("fearJumpHeight", v.fearJumpHeight);
    a.Field("avoidanceTurnDegrees", v.avoidanceTurnDegrees);
    a.Field("huntProbeFrames", v.huntProbeFrames);
    a.Field("huntJumpHeight", v.huntJumpHeight);
    a.Field("huntStopDistance", v.huntStopDistance);
    a.Field("fireEnergyFraction", v.fireEnergyFraction);
    a.Field("shotCharge", v.shotCharge);
    a.Field("spawnRotationStepDegrees", v.spawnRotationStepDegrees);
    a.Field("overflowSpawnOffset", v.overflowSpawnOffset);
}
template<class A> void Visit(EnemyRules& v, A& a) {
    a.Field("openingLevelWeakenedIndex", v.openingLevelWeakenedIndex);
    a.Field("openingLevelAttackScale", v.openingLevelAttackScale);
    a.Field("openingTypes", v.openingTypes);
    a.Field("regularTypes", v.regularTypes);
    a.Field("advancedTypes", v.advancedTypes);
    a.Field("advancedLevelOffsets", v.advancedLevelOffsets);
    a.Field("openingWeakenedPrimary", v.openingWeakenedPrimary);
    a.Field("openingWeakenedSecondary", v.openingWeakenedSecondary);
}
template<class A> void Visit(DifficultyModifiers& v, A& a) {
    a.Field("health", v.health);
    a.Field("healthRegen", v.healthRegen);
    a.Field("energy", v.energy);
    a.Field("energyRegen", v.energyRegen);
}

template<class T, class Enable> struct Codec {
    static Json Write(const T& value) {
        T copy = value;
        Writer writer;
        Visit(copy, writer);
        return writer.result;
    }
    static void Read(const Json& j, T& value, const std::string& path) {
        Reader reader(j, path);
        Visit(value, reader);
        reader.Finish();
    }
};

template<class A> void VisitGroups(GameplayDefinitions& v, A& a) {
    a.Field("tank", v.tank);
    a.Field("projectile", v.projectile);
    a.Field("player", v.player);
    a.Field("combat", v.combat);
    a.Field("enemy", v.enemy);
    a.Field("input", v.input);
    a.Field("joystick", v.joystick);
    a.Field("special", v.special);
    a.Field("landing", v.landing);
    a.Field("enemyBehavior", v.enemyBehavior);
    a.Field("gameCube", v.gameCube);
}
template<class T> void ReadOverride(const Json& j, const char* name, Override<T>& field, const std::string& path) {
    if (j.count(name)) { Decode(j.at(name), field.value, path + "." + name); field.present = true; }
}
Json WriteOverrides(const TankTypeOverride& v) {
    Json j = Json::object();
    if (v.fireIntervalSeconds.present) j["fireIntervalSeconds"] = v.fireIntervalSeconds.value;
    if (v.attack.present) j["attack"] = v.attack.value;
    if (v.fireCost.present) j["fireCost"] = v.fireCost.value;
    if (v.specialChargeCost.present) j["specialChargeCost"] = v.specialChargeCost.value;
    if (v.bounces.present) j["bounces"] = v.bounces.value;
    return j;
}
TankTypeOverride ReadOverrides(const Json& j, const std::string& path) {
    if (!j.is_object()) Fail(path, "expected an object");
    const std::set<std::string> allowed = {"fireIntervalSeconds","attack","fireCost","specialChargeCost","bounces"};
    for (auto it = j.begin(); it != j.end(); ++it)
        if (!allowed.count(it.key())) Fail(path + "." + it.key(), "unknown override");
    TankTypeOverride v;
    ReadOverride(j, "fireIntervalSeconds", v.fireIntervalSeconds, path);
    ReadOverride(j, "attack", v.attack, path);
    ReadOverride(j, "fireCost", v.fireCost, path);
    ReadOverride(j, "specialChargeCost", v.specialChargeCost, path);
    ReadOverride(j, "bounces", v.bounces, path);
    return v;
}
} // namespace

GameplayDefinitions ParseDefinitionJson(const std::string& text) {
    if (text.size() > MaxBytes) Fail("$", "document exceeds 1 MiB");
    // The callback sees keys before DOM insertion, so duplicate keys cannot be lost.
    std::vector<std::set<std::string>> objectKeys;
    Json document;
    try {
        document = Json::parse(text, [&objectKeys](int depth, Json::parse_event_t event, Json& parsed) {
            if (depth > 32) Fail("$", "nesting exceeds 32 levels");
            if (event == Json::parse_event_t::object_start) objectKeys.emplace_back();
            if (event == Json::parse_event_t::key && !objectKeys.back().insert(parsed.get<std::string>()).second)
                Fail("$", "duplicate object key: " + parsed.get<std::string>());
            if (event == Json::parse_event_t::object_end) objectKeys.pop_back();
            return true;
        });
    } catch (const Json::exception& e) { Fail("$", e.what()); }
    Keys(document, {"schemaVersion", "definitions"}, "$");
    int version;
    Decode(document.at("schemaVersion"), version, "$.schemaVersion");
    if (version != 1) Fail("$.schemaVersion", "unsupported version " + std::to_string(version) + "; supported: 1");
    GameplayDefinitions result;
    const auto& j = document.at("definitions");
    Reader reader(j, "$.definitions");
    VisitGroups(result, reader);
    reader.fields.insert("types"); reader.fields.insert("difficulties"); reader.fields.insert("combinations");
    reader.Finish();
    for (auto name : {"types", "difficulties", "combinations"})
        if (!j.count(name)) Fail(std::string("$.definitions.") + name, "required field missing");
    const auto& types = j.at("types");
    Keys(types, {"grey", "red", "blue", "yellow", "purple"}, "$.definitions.types");
    for (int i = 0; i < 5; ++i)
        Decode(types.at(TypeNames[i]), result.types[Types[i]], std::string("$.definitions.types.") + TypeNames[i]);
    const auto& difficulties = j.at("difficulties");
    Keys(difficulties, {"easy", "normal", "hard"}, "$.definitions.difficulties");
    for (int i = 0; i < 3; ++i)
        Decode(difficulties.at(DifficultyNames[i]), result.difficulties[i], std::string("$.definitions.difficulties.") + DifficultyNames[i]);
    const auto& combos = j.at("combinations");
    if (!combos.is_array() || combos.size() > 25) Fail("$.definitions.combinations", "expected at most 25 ordered pairs");
    for (std::size_t i = 0; i < combos.size(); ++i) {
        const std::string path = "$.definitions.combinations[" + std::to_string(i) + "]";
        Keys(combos[i], {"primary", "secondary", "overrides"}, path);
        auto pair = std::make_pair(ReadType(combos[i].at("primary"), path + ".primary"),
                                   ReadType(combos[i].at("secondary"), path + ".secondary"));
        if (!result.combinations.emplace(pair, ReadOverrides(combos[i].at("overrides"), path + ".overrides")).second)
            Fail(path, "duplicate ordered pair");
    }
    ValidateDefinitions(result);
    return result;
}

std::string SerializeDefinitionJson(const GameplayDefinitions& definitions) {
    ValidateDefinitions(definitions);
    GameplayDefinitions copy = definitions;
    Writer writer;
    VisitGroups(copy, writer);
    auto& j = writer.result;
    for (int i = 0; i < 5; ++i) j["types"][TypeNames[i]] = Encode(definitions.types.at(Types[i]));
    for (int i = 0; i < 3; ++i) j["difficulties"][DifficultyNames[i]] = Encode(definitions.difficulties[i]);
    j["combinations"] = Json::array();
    for (const auto& combo : definitions.combinations)
        j["combinations"].push_back({{"primary", TypeName(combo.first.first)},
            {"secondary", TypeName(combo.first.second)}, {"overrides", WriteOverrides(combo.second)}});
    const std::string text = Json({{"schemaVersion", 1}, {"definitions", j}}).dump(2) + "\n";
    if (text.size() > MaxBytes) Fail("$", "export exceeds 1 MiB");
    return text;
}
} // namespace Gameplay
