LiveryLayerPatch for Forza Horizon 2 (Azimuth)
==============================================

Raises the livery editor's layer limits (1,000 and 3,000) up to 20,000.
Patches the game in memory, through the FH2 Mod Platform: the game exe,
azimuth.exe and Extra Options' winmm.dll are not modified.

Install: extract everything into the folder with azimuth.exe, so you get
  DWrite.dll                       FH2 Mod Platform (Azimuth 1.0.1 or later)
  fh2-mods\LiveryLayerPatch.dll    this mod

IMPORTANT: the DLL goes in fh2-mods (the FH2 Mod Platform's folder), NOT in
Extra Options' mods folder. It isn't an Extra Options mod and won't load there.

Uninstall: delete fh2-mods\LiveryLayerPatch.dll.

Settings, in game under Pause > Settings > MODS (both take effect after a
restart), or in LiveryLayerPatch\LiveryLayerPatch.ini:
  Enabled=1      0 keeps the game's own limits
  HoodSidesLimit=20000     hood, left and right side: 3000 to 20000 (game: 3000)
  BumpersWingLimit=20000   front/rear bumper and wing: 1000 to 20000 (game: 1000)
  20000 is the tested value.

Needs FH2 1.0.0.64 with an UNPATCHED forza_durango_release_final.exe. If you
used FH2 Layer Patcher or Studio before, restore the exe with that tool first.

Log: fh2-mods\platform.log (lines starting [LiveryLayerPatch]).

The platform includes MinHook (BSD 2-Clause): see LICENSE-MinHook.txt.
