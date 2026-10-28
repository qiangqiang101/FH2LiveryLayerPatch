// LiveryLayerPatch: raises Forza Horizon 2's livery layer limits (1,000 and
// 3,000) to 20,000, or a limit of the player's choosing, in memory, through
// the FH2 Mod Platform.
//
// At 20,000 it makes the same four edits as FH2 Layer Patcher 0.4 (profile
// fh2-10064-azimuth102-extra032-20000), but to the mapped game instead of
// forza_durango_release_final.exe on disk. The exe, azimuth.exe and Extra
// Options' winmm.dll stay untouched, so the hash edits that patcher had to
// make to the launcher and to Extra Options aren't needed. Remove the DLL
// (or switch it off on the Mods page) and the game is stock again.
#include "fh2mod.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

namespace {

const FH2ModApi* g_api = nullptr;
FH2Mod* g_mod          = nullptr;
std::wstring g_ini;

// The patcher's edits, as offsets into forza_durango_release_final.exe
// (1.0.0.64, SHA-256 3ec04c61...ccb7f0a3): little-endian 16-bit limits,
// 0x03E8 = 1,000 and 0x0BB8 = 3,000. The three 3,000s are the hood and the
// two sides, the three 1,000s the bumpers and the wing; each group gets its
// own limit.
// The limits are 16-bit, and 20,000 is what FH2 Layer Patcher shipped and
// was tested in game; going below a group's own limit would lower it.
struct Group {
    const char* key;   // setting and ini key
    const char* label; // Mods page
    const char* description;
    int game_limit;    // also the lowest setting
    int limit;         // chosen, read from the ini
};
constexpr int kMaxLimit     = 20000;
constexpr int kDefaultLimit = 20000;
Group g_groups[] = {
    {"HoodSidesLimit", "Hood and sides layer limit",
     "Layers allowed on the hood, left side and right side: 3,000 to 20,000 (game: 3,000). 20,000 is the tested value.",
     3000, kDefaultLimit},
    {"BumpersWingLimit", "Bumpers and wing layer limit",
     "Layers allowed on the front bumper, rear bumper and wing: 1,000 to 20,000 (game: 1,000). 20,000 is the tested value.",
     1000, kDefaultLimit},
};
Group& g_hood_sides   = g_groups[0];
Group& g_bumpers_wing = g_groups[1];

struct Edit {
    const char* name;
    uint32_t file_offset;
    uint32_t count; // 16-bit values
    const Group& group;
};
const Edit kEdits[] = {
    {"Bumpers and wing layer limit (1)", 0x2BF7EF, 1, g_bumpers_wing},
    {"Bumpers and wing layer limit (2)", 0x2BF7F9, 2, g_bumpers_wing},
    {"Hood and sides layer limit (1)", 0x2BF7FE, 1, g_hood_sides},
    {"Hood and sides layer limit (2)", 0x2BF808, 2, g_hood_sides},
};

int clamp_limit(const Group& g, int v) { return v < g.game_limit ? g.game_limit : v > kMaxLimit ? kMaxLimit : v; }

void fill(unsigned char* out, uint32_t count, uint16_t value) {
    for (uint32_t i = 0; i < count; ++i) {
        out[2 * i]     = static_cast<unsigned char>(value);
        out[2 * i + 1] = static_cast<unsigned char>(value >> 8);
    }
}

void logf(const char* fmt, ...) {
    char line[256];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    g_api->log(g_mod, line);
}

// Where a file offset of the exe is in the mapped game, from the section
// headers Azimuth maps with it; NULL if no section holds those bytes.
unsigned char* file_offset_to_address(unsigned char* base, uint32_t offset, uint32_t size) {
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;
    const IMAGE_SECTION_HEADER* s = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++s) {
        if (offset >= s->PointerToRawData && offset + size <= s->PointerToRawData + s->SizeOfRawData)
            return base + s->VirtualAddress + (offset - s->PointerToRawData);
    }
    return nullptr;
}

DWORD WINAPI patch_game(LPVOID) {
    // After Extra Options has patched the game, before the game runs.
    auto* base = static_cast<unsigned char*>(g_api->wait_for_code(0xFFFFFFFFu));
    if (!base) {
        g_api->log(g_mod, "the game never became ready to patch; nothing changed");
        return 0;
    }

    // Every edit or none: check them all before switching any on.
    FH2Hook* hooks[sizeof kEdits / sizeof kEdits[0]] = {};
    int made = 0;
    for (const Edit& e : kEdits) {
        const uint32_t size = 2 * e.count;
        unsigned char before[4], after[4];
        fill(before, e.count, static_cast<uint16_t>(e.group.game_limit));
        fill(after, e.count, static_cast<uint16_t>(e.group.limit));
        unsigned char* at = file_offset_to_address(base, e.file_offset, size);
        if (!at) {
            logf("%s: file offset 0x%X isn't in any section of the mapped game", e.name, e.file_offset);
            break;
        }
        if (std::memcmp(at, before, size) != 0 && at[0] == 0x20 && at[1] == 0x4E) {
            logf("%s: game+0x%X already holds 20000. forza_durango_release_final.exe looks patched on disk "
                 "(FH2 Layer Patcher or Studio): restore it with that tool, this mod does the same in memory",
                 e.name, static_cast<unsigned>(at - base));
            break;
        }
        hooks[made] = g_api->patch_bytes(g_mod, e.name, at, before, after, size);
        if (!hooks[made]) break; // the platform logs why
        ++made;
    }
    if (made != static_cast<int>(sizeof kEdits / sizeof kEdits[0])) {
        g_api->log(g_mod, "not every layer limit could be patched, so none were: the game keeps its own limits");
        return 0;
    }
    if (!g_api->enable_hooks(g_mod, nullptr)) {
        g_api->disable_hooks(g_mod, nullptr);
        g_api->log(g_mod, "switching the patches on failed; switched them all back off");
        return 0;
    }
    logf("livery layer limits raised: hood and sides %d, bumpers and wing %d", g_hood_sides.limit,
         g_bumpers_wing.limit);
    return 0;
}

void on_enabled_changed(void*, const char*, int value) {
    WritePrivateProfileStringW(L"Settings", L"Enabled", value ? L"1" : L"0", g_ini.c_str());
}

void on_limit_changed(void* user, const char*, float value) {
    const Group& g = *static_cast<const Group*>(user);
    wchar_t key[32], text[16];
    std::swprintf(key, 32, L"%hs", g.key);
    std::swprintf(text, 16, L"%d", clamp_limit(g, static_cast<int>(value + 0.5f)));
    WritePrivateProfileStringW(L"Settings", key, text, g_ini.c_str());
}

} // namespace

FH2MOD_EXPORT int fh2mod_init(const FH2ModApi* api) {
    if (api->api_version < 7) return 1; // patch_bytes: platform 2.6.0
    g_api = api;

    FH2ModInfo info{};
    info.size             = sizeof info;
    info.name             = "LiveryLayerPatch";
    info.version          = LIVERY_LAYER_PATCH_VERSION;
    info.github_repo      = "qiangqiang101/FH2LiveryLayerPatch";
    info.platform_version = FH2MOD_PLATFORM_VERSION;
    info.check_updates    = 1;
    info.description      = "Raises the livery editor's layer limits (up to 20,000), in memory: no game files are changed.";
    g_mod                 = api->register_mod(&info);
    if (!g_mod) return 1;

    // Settings in LiveryLayerPatch\LiveryLayerPatch.ini next to azimuth.exe.
    std::wstring dir = std::wstring(api->azimuth_dir) + L"\\LiveryLayerPatch";
    CreateDirectoryW(dir.c_str(), nullptr);
    g_ini              = dir + L"\\LiveryLayerPatch.ini";
    const int enabled = GetPrivateProfileIntW(L"Settings", L"Enabled", 1, g_ini.c_str()) != 0;
    for (Group& g : g_groups) {
        wchar_t key[32];
        std::swprintf(key, 32, L"%hs", g.key);
        g.limit = clamp_limit(g, static_cast<int>(GetPrivateProfileIntW(L"Settings", key, kDefaultLimit, g_ini.c_str())));
    }

    // A restart, not a live switch: the game may size its layer storage from
    // these limits when it starts.
    FH2Setting row{};
    row.size          = sizeof row;
    row.key           = "Enabled";
    row.label         = "Raise livery layer limit";
    row.description   = "Raises the livery editor's layer limits (hood and sides 3,000, bumpers and wing 1,000) "
                        "to the limits below. Off: the game's own limits.";
    row.value         = enabled;
    row.needs_restart = 1;
    row.on_change     = &on_enabled_changed;
    api->add_setting(g_mod, &row);

    for (Group& g : g_groups) {
        FH2Setting slider{};
        slider.size          = sizeof slider;
        slider.key           = g.key;
        slider.label         = g.label;
        slider.description   = g.description;
        slider.needs_restart = 1;
        slider.user          = &g;
        slider.kind          = FH2_SETTING_SLIDER;
        slider.min           = static_cast<float>(g.game_limit);
        slider.max           = kMaxLimit;
        slider.step          = 1000;
        slider.slider_value  = static_cast<float>(g.limit);
        slider.on_slide      = &on_limit_changed;
        api->add_setting(g_mod, &slider);
    }

    if (!enabled) {
        api->log(g_mod, "switched off in LiveryLayerPatch.ini; the game keeps its own layer limits");
        return 0;
    }
    if (HANDLE t = CreateThread(nullptr, 0, patch_game, nullptr, 0, nullptr)) CloseHandle(t);
    return 0;
}
