#pragma once

#include <cstdint>

enum ObjectType
{
    NONE,
    ITEM,
    CONTAINER,
    UNIT,
    PLAYER,
    GAMEOBJECT,
    DYNAMICOBJECT,
    CORPSE
};

enum Offsets
{
    FUN_OBJECT_POINTER =        0x464870,
    FUN_IS_IN_WORLD =           0xB4B424,
    FUN_LUA_CFUNCTION =         0x6F3070,
    FUN_LUA_TOSTRING =          0x6F3690,
    FUN_RIGHT_CLICK_UNIT =      0x60BEA0,
    FUN_RIGHT_CLICK_OBJECT =    0x5F8660,
    FUN_SET_TARGET =            0x493540,
    FUN_REGISTER_FUNCTION =     0x704120,   // FrameScript_RegisterFunction(name, fn): into the current Lua state
    VISIBLE_OBJECTS =           0xB41414,
};

// Gameobject descriptor fields, as byte offsets from the descriptor pointer at object + 0x8.
// OBJECT_END is field 0x06; OBJECT_FIELD_CREATED_BY is OBJECT_END + 0x00 and
// GAMEOBJECT_TYPE_ID is OBJECT_END + 0x0F (UpdateFields.h, 1.12.1).
enum GameObjectFields
{
    GO_CREATED_BY =             0x18,
    GO_TYPE_ID =                0x54,
};

enum GameObjectType
{
    GO_TYPE_FISHINGNODE =       17,
};

// The client casts the bobber 10 to 20 yards out. The server finds fishing pools within 20 yards of it.
constexpr float FISHING_BOBBER_RANGE = 30.0f;

// The key reaches only objects in this cone in front of the player (total angle). The bobber is not limited by it.
constexpr float INTERACT_CONE_DEGREES = 145.0f;

typedef int(__fastcall* LUA_CFUNCTION)(void* L);
typedef const char* (__fastcall* LUA_TOSTRING)(void* L, int index);
typedef void(__thiscall* FUN_ONRIGHTCLICK)(uint32_t pointer, int autoloot);
typedef void(__fastcall* FUN_REGISTERFUNCTION)(const char* name, LUA_CFUNCTION function);

typedef struct 
{
    float y;
    float x;
    float z;
} C3Vector;

namespace Game
{
    uint32_t __stdcall GetObjectPointer(uint64_t guid);

    C3Vector GetObjectPosition(uint32_t pointer);

    int GetUnitHealth(uint32_t unit);

    C3Vector GetUnitPosition(uint32_t unit);

    float GetUnitFacing(uint32_t unit);

    void Interact(uint32_t pointer, int autoloot, int fun_ptr);

    bool IsUnitLootable(uint32_t unit);
    bool IsUnitSkinnable(uint32_t unit);

    uint64_t GetObjectCreator(uint32_t object);
    uint32_t GetObjectType(uint32_t object);

    void SetTarget(uint64_t guid);

    inline bool IsInWorld() { return *(char*)Offsets::FUN_IS_IN_WORLD; }
}

namespace Lua
{
    const char* ToString(void* L, int index);

    int GetTop(void* L);
}