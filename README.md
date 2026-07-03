# gbs-reloadSceneTilesetPlugin

**Version 4.3.0 — Requires GB Studio ≥ 4.3.0**

A GB Studio engine plugin that adds a single event for re-uploading the current scene's background tileset from ROM back into VRAM, without performing a full scene change. This is useful for restoring tile graphics after runtime VRAM modifications — such as those made by the replaceTilesetTilesPlugin, the SubmappingExPlugin, or any other event that writes directly to background tile slots.

On CGB hardware both VRAM bank 0 and VRAM bank 1 are restored.

---

## Table of Contents

1. [Concepts](#concepts)
2. [Project Setup](#project-setup)
3. [How to Use](#how-to-use)
4. [Technicalities and Restrictions](#technicalities-and-restrictions)
5. [Events Reference](#events-reference)
6. [Inner Workings](#inner-workings)
7. [Memory Footprint](#memory-footprint)

---

## Concepts

### Why a Tileset Reload Is Needed

GB Studio loads the current scene's background tileset into VRAM once when the scene starts. Any plugin or event that subsequently writes new tile bitmaps into VRAM slots (tile replacement, submapping, animated tiles) permanently overwrites that data until the scene is reloaded. If the game needs to revert those tile changes — for example, returning a room to its default appearance without a scene transition — the original tileset data must be re-uploaded from ROM.

This plugin replicates exactly what the engine does at scene load time, but as a callable script event, so it can be triggered at any point during gameplay.

### What Is Restored

The event reads the current scene's `background_t` structure from ROM, extracts the tileset far pointer, and re-uploads all tile bitmaps to VRAM. On CGB hardware, both the DMG-compatible tileset (VRAM bank 0) and the CGB-specific tileset (VRAM bank 1) are restored.

The **tilemap** (which tile indices are drawn at which positions on the background map) is **not** touched by this event. Only the pixel bitmaps stored in the VRAM tile slots are overwritten.

---

## Project Setup

1. Copy the plugin folder into your GB Studio project's `plugins/` directory.
2. No additional configuration, engine fields, or compatibility variants are required.

---

## How to Use

Add a **Reload current scene tileset** event to any script at the point where you want to restore the scene's original tile graphics:

- After a sequence of **Replace Tileset Tiles** events whose effect should be undone.
- After returning from a submapped or tile-replaced view to the base scene appearance.
- In the **On pop** subscript of a SceneStackEx push event, to restore VRAM after returning from a stacked scene.

The event has no fields — simply placing it in the script is sufficient.

---

## Technicalities and Restrictions

### Tilemap Is Not Reloaded

This event only restores tile bitmap data in VRAM. It does not reload the background tilemap (the grid of tile indices drawn on screen). If any script has written new tile index values to the tilemap (e.g. via the SubmappingExPlugin or a Set BKG Tile event), those tilemap changes remain after this event.

### UI Tiles Are Not Reloaded

The engine reserves the uppermost VRAM tile slots for the UI font and dialogue frame. This event follows the same allocation logic as the engine's own `load_bkg_tileset` and does not write into those reserved slots (assuming the tileset stays within its normal size limits).

### Large Tilesets: Overflow into Sprite VRAM

For tilesets with more than 256 tiles (only applicable to 360-tile logo scenes), tiles beyond index 255 are loaded into the sprite tile area (`SetBankedSpriteData`). This matches the engine's own behaviour at scene load time.

### CGB Dual-Bank Reload

On CGB hardware, if the background has a CGB-specific tileset (`cgb_tileset.ptr` is non-null), the event switches to VRAM bank 1 (`VBK_REG = 1`), reloads that tileset, then restores `VBK_REG` to 0. On DMG hardware the CGB block is compiled out.

### Does Not Affect Sprites or Actors

Sprite VRAM (OBJ tiles for actors and projectiles) is not touched, except in the rare 360-tile logo overflow case described above.

### No Engine Files Modified

This plugin only adds a new engine source file (`reload_current_scene_tileset.c`). No existing GB Studio engine files are patched.

---

## Events Reference

### Reload Current Scene Tileset

**Event ID:** `EVENT_RELOAD_CURRENT_SCENE_TILESET`  
**Group:** Scene

Re-uploads all background tile bitmaps for the current scene from ROM into VRAM. On CGB, restores both VRAM bank 0 and bank 1.

This event has no configurable fields.

---

## Inner Workings

### `reload_current_scene_tileset` Native Function

```c
void reload_current_scene_tileset(SCRIPT_CTX * THIS) OLDCALL BANKED {
    scene_t scn;
    MemcpyBanked(&scn, current_scene.ptr, sizeof(scn), current_scene.bank);

    background_t bkg;
    MemcpyBanked(&bkg, scn.background.ptr, sizeof(bkg), scn.background.bank);

    reload_bkg_tileset(bkg.tileset.ptr, bkg.tileset.bank);

#ifdef CGB
    if ((_is_CGB) && (bkg.cgb_tileset.ptr)) {
        VBK_REG = 1;
        reload_bkg_tileset(bkg.cgb_tileset.ptr, bkg.cgb_tileset.bank);
        VBK_REG = 0;
    }
#endif
}
```

**Step by step:**

1. `current_scene` is the global far pointer maintained by `data_manager.c` that always points to the ROM definition of the currently active scene. `MemcpyBanked` reads the `scene_t` struct from ROM into a local stack variable.
2. The `scene_t` contains a far pointer to its `background_t`. A second `MemcpyBanked` reads that struct.
3. `reload_bkg_tileset` is called with the DMG tileset far pointer.
4. On CGB, if the background has a CGB tileset (`cgb_tileset.ptr` is non-null), `VBK_REG` is set to 1 to select VRAM bank 1, the CGB tileset is loaded, then `VBK_REG` is restored to 0.

### `reload_bkg_tileset` Allocation Logic

The local `static` helper mirrors the engine's own `load_bkg_tileset` function, applying the same three-region allocation scheme:

```
Tiles 0 – 127     → BKG VRAM slots 0–127     (SetBankedBkgData at index 0)
Tiles 128 – 191   → BKG VRAM slots 192–(192-n) if n_tiles < 65, else slots 128+
                    (ALLOC_BKG_TILES_TOWARDS_SPR compile flag controls alignment)
Tiles 192 – 255   → BKG VRAM slots 128–255   (SetBankedBkgData at index 128)
Tiles 256+        → Sprite VRAM slots 0+      (SetBankedSpriteData, logo scenes only)
```

The purpose of the `ALLOC_BKG_TILES_TOWARDS_SPR` path for small second chunks is to pack tiles toward slot 192 and leave the upper UI-reserved area free, matching the engine's tile allocation strategy introduced in GB Studio 4.x.

Each `SetBankedBkgData` call performs the necessary ROM bank switch to read the tile pixel data before writing it into VRAM.


---

## Memory Footprint

Measured against the stock GB Studio **4.3.0-e1** engine (per-file SDCC compile with GB Studio's build flags, default engine settings). Values are the plugin's *delta* versus the stock engine; DMG build, with CGB noted where it differs. ROM cost lands in banked ROM (GB Studio's autobanker spreads it across switchable banks); using the plugin's events additionally compiles a few bytes of GBVM script per call into your project's script banks.

| | Cost |
|---|---|
| WRAM | +0 bytes |
| ROM | +284 bytes (DMG) / +314 bytes (CGB) |

- **WRAM:** no change.
- **Engine WRAM headroom:** the stock GB Studio 4.3.0 engine leaves about **854 bytes** of WRAM free (usable engine WRAM is 7,776 bytes at 0xC0A0–0xDF00; the stock engine uses 6,922 bytes). With this plugin installed roughly **854 bytes** remain. This figure does not depend on how many global variables your project defines: the script memory array has a fixed size of VM_HEAP_SIZE + (VM_MAX_CONTEXTS × VM_CONTEXT_STACK_SIZE) words — 768 + 16 × 64 = 1,792 words (3,584 bytes) with stock engine settings.
- **SRAM:** not used.
