// dllmain.cpp : Defines the entry point for the DLL application.
#include "Game.h"
#include "MinHook.h"

#include <cmath>

FUN_REGISTERFUNCTION p_original_RegisterFunction = NULL;
FUN_REGISTERFUNCTION p_RegisterFunction = reinterpret_cast<FUN_REGISTERFUNCTION>(Offsets::FUN_REGISTER_FUNCTION);

static float distance3D(C3Vector v1, C3Vector v2) {
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    float dz = v2.z - v1.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

// True when the object is inside the INTERACT_CONE_DEGREES cone in front of the player.
// C3Vector names its first float y, but the client stores world X there (unit + 0x9B8 is X).
// Facing is measured from world +X toward world +Y, so the angle is atan2(world dY, world dX),
// which is atan2(dx, dy) in C3Vector names.
static bool isInFront(C3Vector player, float facing, C3Vector object)
{
    const float pi = 3.14159265f;

    float worldDX = object.y - player.y;
    float worldDY = object.x - player.x;

    float difference = atan2f(worldDY, worldDX) - facing;
    while (difference > pi) difference -= 2.0f * pi;
    while (difference < -pi) difference += 2.0f * pi;

    return fabsf(difference) <= INTERACT_CONE_DEGREES / 2.0f * pi / 180.0f;
}

static uint32_t InteractNearest(int autoloot)
{
    if (!Game::IsInWorld()) return 0;

    uint32_t objects = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
    uint32_t currentObject = *reinterpret_cast<uint32_t*>(objects + 0xAC);
    uint32_t candidate = -1;

    uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
    uint32_t player = Game::GetObjectPointer(playerGUID);

    C3Vector pPos = Game::GetUnitPosition(player);
    float pFacing = Game::GetUnitFacing(player);
    C3Vector oPos;

    float bestDistance = 1000.0f;

    // The player's own fishing bobber. It lands out of the 5 yard reach, so it has its own range,
    // and it wins over every other candidate.
    uint32_t bobber = 0;

    while (currentObject != 0 && (currentObject & 1) == 0)
    {
        uint64_t guid = *reinterpret_cast<uint64_t*>(currentObject + 0x30);
        uint32_t pointer = Game::GetObjectPointer(guid);
        uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);

        if (type == ObjectType::UNIT)
        {
            // UNIT_FIELD_SUMMONEDBY. Only units have it: on a gameobject, descriptor + 0x30 is the rotation.
            uint64_t summonedByGUID = *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint32_t*>(pointer + 0x8) + 0x30);
            uint32_t summonedBy = Game::GetObjectPointer(summonedByGUID);

            if (summonedByGUID != 0 && summonedBy != 0)
            {
                uint32_t owner = *reinterpret_cast<uint32_t*>(summonedBy + 0x14);
                if (owner == ObjectType::PLAYER)
                {
                    currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                    continue;
                }
            }

            oPos = Game::GetUnitPosition(currentObject);
        }
        else if (type == ObjectType::GAMEOBJECT)
        {
            oPos = Game::GetObjectPosition(currentObject);

            if (Game::GetObjectType(pointer) == GameObjectType::GO_TYPE_FISHINGNODE)
            {
                // The server refuses a bobber that another player cast, so skip those.
                if (Game::GetObjectCreator(pointer) == playerGUID && distance3D(oPos, pPos) <= FISHING_BOBBER_RANGE)
                {
                    bobber = currentObject;
                }

                currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                continue;
            }
        }
        else
        {
            currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
            continue;
        }

        float distance = distance3D(oPos, pPos);
        if (distance <= 5.0 && distance < bestDistance && isInFront(pPos, pFacing, oPos))
        {
            if (type == ObjectType::UNIT)
            {
                if (Game::GetUnitHealth(currentObject) == 0 && (Game::IsUnitLootable(currentObject) || Game::IsUnitSkinnable(currentObject)))
                {
                    bestDistance = distance;
                    candidate = currentObject;
                }
                else if (Game::GetUnitHealth(currentObject) > 0)
                {
                    bestDistance = distance;
                    candidate = currentObject;
                }
            }
            else if (type == ObjectType::GAMEOBJECT)
            {
                bestDistance = distance;
                candidate = currentObject;
            }
        }

        currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
    }

    if (bobber != 0)
    {
        uint64_t bobberGUID = *reinterpret_cast<uint64_t*>(bobber + 0x30);
        Game::Interact(Game::GetObjectPointer(bobberGUID), autoloot, Offsets::FUN_RIGHT_CLICK_OBJECT);
        return 1;
    }

    if (candidate == -1) return 0;

    uint64_t candidateGUID = *reinterpret_cast<uint64_t*>(candidate + 0x30);
    int candidatePointer = Game::GetObjectPointer(candidateGUID);
    uint32_t candidateType = *reinterpret_cast<uint32_t*>(candidatePointer + 0x14);

    if (candidateType == ObjectType::UNIT)
    {
        Game::SetTarget(candidateGUID);
        Game::Interact(candidate, autoloot, Offsets::FUN_RIGHT_CLICK_UNIT);
    }
    else
    {
        Game::Interact(candidatePointer, autoloot, Offsets::FUN_RIGHT_CLICK_OBJECT);
    }

    return 1;
}

// InteractNearest(autoloot), called from Interact.lua. autoloot is 1 or 0, and 0 when it is left out.
static int __fastcall Script_InteractNearest(void* L)
{
    int autoloot = 0;
    if (Lua::GetTop(L) >= 1)
    {
        const char* arg = Lua::ToString(L, 1);
        autoloot = (arg != nullptr && strcmp(arg, "1") == 0) ? 1 : 0;
    }

    InteractNearest(autoloot);
    return 0;
}

// The client registers its world functions one at a time. InteractNearest goes in right after UnitXP,
// so it lands in the world Lua state and not the login screen's. Nothing else hooks UnitXP itself any more,
// so UnitXP_SP3 has it to itself.
static void __fastcall detoured_RegisterFunction(const char* name, LUA_CFUNCTION function)
{
    p_original_RegisterFunction(name, function);

    if (name != nullptr && strcmp(name, "UnitXP") == 0)
    {
        p_original_RegisterFunction("InteractNearest", &Script_InteractNearest);
    }
}

// FrameScript_RegisterFunction as it ships (56 57 8B F9 E8), or a jmp that another DLL put there.
static bool RegisterFunctionLooksRight()
{
    const uint8_t* head = reinterpret_cast<const uint8_t*>(Offsets::FUN_REGISTER_FUNCTION);
    const uint8_t stock[] = { 0x56, 0x57, 0x8B, 0xF9, 0xE8 };
    return memcmp(head, stock, sizeof(stock)) == 0 || head[0] == 0xE9;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);

            // An unknown exe: load, do nothing. Interact.lua then shows its warning.
            if (!RegisterFunctionLooksRight())
            {
                break;
            }

            if (MH_Initialize() != MH_OK)
            {
                return FALSE;
            }

            if (MH_CreateHook(p_RegisterFunction, &detoured_RegisterFunction, reinterpret_cast<LPVOID*>(&p_original_RegisterFunction)) != MH_OK)
            {
                return FALSE;
            }

            if (MH_EnableHook(p_RegisterFunction) != MH_OK)
            {
                return FALSE;
            }

            break;

        case DLL_PROCESS_DETACH:
            MH_Uninitialize();
            break;
    }

    return TRUE;
}