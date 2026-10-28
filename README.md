# LiveryLayerPatch

A [FH2 Mod Platform](https://github.com/Trinculo54/FH2-Mod-Platform) mod for Forza Horizon 2 (PC, via Azimuth) that raises the livery editor's layer limits:

| Panels | Game limit | With this mod |
|---|---|---|
| Hood, left side, right side | 3,000 | up to 20,000 |
| Front bumper, rear bumper, wing | 1,000 | up to 20,000 |

The game is patched **in memory** while it runs. Nothing on disk is modified: not `forza_durango_release_final.exe`, not `azimuth.exe`, not Extra Options' `winmm.dll`. To go back to stock, delete the DLL.

## Requirements

- Forza Horizon 2 1.0.0.64 running through Azimuth 1.0.3 or later
- An **unpatched** `forza_durango_release_final.exe`. If you used FH2 Layer Patcher or FH2 Vinyl Studio to patch the exe, restore it with that tool first. The mod detects a patched exe and won't apply.

## Install

1. Download `LiveryLayerPatch-<version>.zip` from [Releases](../../releases).
2. Extract everything into the folder that contains `azimuth.exe`, keeping the folders as they are in the zip:

   ```
   <Azimuth folder>\
   ├── azimuth.exe
   ├── DWrite.dll                       FH2 Mod Platform
   └── fh2-mods\
       └── LiveryLayerPatch.dll         this mod
   ```

> [!IMPORTANT]
> `LiveryLayerPatch.dll` goes in **`fh2-mods`**, the FH2 Mod Platform's folder. It is **not** an Extra Options mod: don't put it in Extra Options' `mods` folder, where it won't be loaded. If `fh2-mods` doesn't exist yet, the zip creates it.

3. Start the game. `fh2-mods\platform.log` should contain a line like:

   ```
   [LiveryLayerPatch] livery layer limits raised: hood and sides 20000, bumpers and wing 20000
   ```

To uninstall, delete `fh2-mods\LiveryLayerPatch.dll`.

## Settings

In game under **Pause > Settings > MODS**, or in `LiveryLayerPatch\LiveryLayerPatch.ini` next to `azimuth.exe`. Changes take effect after restarting the game.

| Setting | ini key | Range | Default |
|---|---|---|---|
| Raise livery layer limit | `Enabled` | 0 / 1 | 1 |
| Hood and sides layer limit | `HoodSidesLimit` | 3,000 – 20,000 | 20,000 |
| Bumpers and wing layer limit | `BumpersWingLimit` | 1,000 – 20,000 | 20,000 |

20,000 is the value the original patcher shipped and tested in game.

## How it works

It changes the same six layer-limit values as the original patcher: three 3,000s for the hood and sides, and three 1,000s for the bumpers and wing, at four offsets in the exe. Once the game has been loaded into memory and Extra Options has applied its own patches, the mod finds those offsets in the loaded game and writes the new limits through the platform's `patch_bytes`. Every location's original bytes are checked first, and if any one doesn't match, none are changed. The reason goes in `platform.log`.

Because the exe on disk is never changed, the original patcher's extra edits aren't needed. It had to update the exe's hash inside `azimuth.exe` and Extra Options so they would accept the modified exe.

## Building

You need CMake, Ninja and [llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases) (Visual Studio with CMake also works).

```bash
git clone --recursive <this repo>
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target dist
```

`build/dist` is then laid out as it goes in the Azimuth folder. On Linux, add `-DCMAKE_TOOLCHAIN_FILE=platform/cmake/mingw-w64-x86_64.cmake`.

GitHub Actions ([build.yaml](.github/workflows/build.yaml)) builds when a release is published, which attaches the zip to that release, or when started by hand. The release tag must match `VERSION` in [CMakeLists.txt](CMakeLists.txt).

## Credits

- **Alexander**: the original FH2 Layer Patcher source code and its byte profile, from his **FH2 Vinyl Studio** (0.69). The offsets, values and the finding that these are the livery layer limits are his work; this mod applies them in memory instead of to the exe on disk.
- [FH2 Mod Platform](https://github.com/Trinculo54/FH2-Mod-Platform) by Trinculo54 (MIT), which includes [MinHook](https://github.com/TsudaKageyu/minhook) (BSD 2-Clause).
- [Forza Horizon 2 Extra Options](https://github.com/RokkuDayo/forzahorizon2_extraoptions) by RokkuDayo.

The original patcher is released under the MIT License, "Copyright (c) 2026 FH2 Layer Patcher contributors".

Unofficial fan-made software; not affiliated with Playground Games, Turn 10 or Microsoft.
