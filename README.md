## Interact

Interact is a mod that allows you to bind a key which you can then use to interact with the world around you. Using the key you will be able to pick herbs, mine veins, loot enemies, catch fish and much more without the need to use the mouse to target the object.

The mod works by comparing the distances of visible objects that are close to the player. The best candidate for interaction is then selected on a number of conditions (closest, is lootable, is skinnable).

This is a fork of [vargv666/Interact](https://github.com/vargv666/Interact), which is luskanek's Interact. It adds fishing and its own Lua function.

### Fishing

Cast, wait for the splash, then press the key. The mod uses your own bobber up to 30 yards away, ahead of every other object. It skips bobbers that other players cast.

If you press the key before the splash, the cast ends. Clicking the bobber does the same.

### Installation

> [!NOTE]
> The mod is intended to be used with [VanillaFixes](https://github.com/hannesmann/vanillafixes). Install VanillaFixes first before proceeding with the installation of Interact.

1. Download `Interact.zip` from the [latest release](https://github.com/aloofbit/InteractNew/releases/latest).
2. Extract the downloaded `.zip` file
3. Move the extracted contents to your World of Warcraft directory (overwrite if needed)
4. In your World of Warcraft directory, open `dlls.txt` (included with VanillaFixes) and add `Interact.dll` on a new line at the end of the `dlls.txt` file
5. Launch the game using `VanillaFixes.exe`, log in and check if your Add-ons list contains the `Interact` add-on

If you've done everything correctly, you should be able to find two new keybindings called 'Interact' and 'Interact (auto-loot)' in your keybindings menu (they should be located at the end of the keybindings list).

To interact with a nearby object, press the associated key.

### Macros

`/interact` does the same as the 'Interact' key. `/interact loot` does the same as 'Interact (auto-loot)'.

### How it loads

The DLL adds one Lua function, `InteractNearest(autoloot)`. It does not hook `UnitXP`, so it works next to UnitXP_SP3.

The client adds its world functions to Lua one at a time through `FrameScript_RegisterFunction` (`0x704120`). The DLL hooks that function and adds `InteractNearest` right after `UnitXP`. Thus the function goes into the world Lua state and not into the login screen's state. If the first bytes at `0x704120` are not the stock `56 57 8B F9 E8` or another mod's `jmp`, the DLL does nothing, and the addon shows a warning.

The bobber is found by two gameobject fields: `GAMEOBJECT_TYPE_ID` (17 is a fishing node) and `OBJECT_FIELD_CREATED_BY` (the player who cast it). Their byte offsets are in `Game.h`.

### Building

The DLL is 32-bit, because the 1.12 client is 32-bit.

```
git submodule update --init
cmake -B build -A Win32
cmake --build build --config Release
```

The build writes `Interact.dll` to the repo root.

### Credits

[luskanek](https://github.com/luskanek) wrote Interact. [allfoxwy](https://github.com/allfoxwy) and [Zz9uk3](https://github.com/Zz9uk3) as their repositories provided to be very helpful when creating this mod.
