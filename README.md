# gbs-reloadSceneTilesetPlugin

**Version 4.3.0 — Requires GB Studio ≥ 4.3.0**

A GB Studio engine plugin that adds a single event for re-uploading the current scene's background tileset from ROM back into VRAM, without performing a full scene change. This is useful for restoring tile graphics after runtime VRAM modifications — such as those made by the replaceTilesetTilesPlugin, the SubmappingExPlugin, or any other event that writes directly to background tile slots.

On Game Boy Color hardware both VRAM banks are restored.

---

## Table of Contents

1. [Concepts](#concepts)
2. [Project Setup](#project-setup)
3. [Size Limits and Restrictions](#size-limits-and-restrictions)
4. [Events Reference](#events-reference)
5. [Memory Footprint](#memory-footprint)
6. [Bank 0 (HOME) Usage](#bank-0-home-usage)
7. [Changelog](#changelog)

---

## Concepts

### Why a tileset reload is needed

GB Studio loads the current scene's background tileset into VRAM once, when the scene starts. Any plugin or event that later writes new tile bitmaps into VRAM slots — tile replacement, submapping, animated tiles — permanently overwrites that data until the scene is reloaded.

If the game needs to revert those tile changes, for example returning a room to its default appearance without a scene transition, the original tileset data has to be re-uploaded from ROM. This plugin does exactly what the engine does at scene load time, but as a callable script event that can fire at any point during gameplay.

### What is restored

Only the **pixel bitmaps** stored in the VRAM tile slots. On Game Boy Color, both the DMG tileset and the CGB-specific tileset are restored.

The **tilemap** — which tile index is drawn at each position on the background map — is not touched.

---

## Project Setup

1. Copy the plugin folder into your GB Studio project's `plugins/` directory. No additional configuration, engine fields, or compatibility variants are required.
2. Add a **Reload current scene tileset** event to any script at the point where you want to restore the scene's original tile graphics. Typical places:
   - after a sequence of **Replace Tileset Tiles** events whose effect should be undone;
   - after returning from a submapped or tile-replaced view to the base scene appearance;
   - in the **On pop** subscript of a SceneStackEx push event, to restore VRAM after returning from a stacked scene.

The event has no fields — placing it in the script is enough.

---

## Size Limits and Restrictions

### The tilemap is not reloaded

This event restores tile bitmap data only. If a script has written new tile *index* values to the background tilemap — via the SubmappingExPlugin or a Set BKG Tile event, for instance — those changes remain afterwards.

### UI tiles are not reloaded

The engine reserves the uppermost VRAM tile slots for the UI font and dialogue frame. This event follows the engine's own allocation and does not write into those reserved slots, assuming the tileset stays within its normal size limits.

### Sprites and actors are unaffected

Sprite VRAM is not touched, with one exception: for tilesets of more than 256 tiles — which in practice only means 360-tile logo scenes — tiles beyond index 255 are loaded into the sprite tile area, matching the engine's own behaviour at scene load.

### No engine files modified

The plugin only adds a new engine source file, so it has no compatibility conflicts with other engine plugins.

---

## Events Reference

---

### Reload current scene tileset

**`EVENT_RELOAD_CURRENT_SCENE_TILESET`** — group: **Scene**

Re-uploads all background tile bitmaps for the current scene from ROM into VRAM. On Game Boy Color, restores both VRAM banks.

This event has no configurable fields.

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

---

<!-- BANK0:BEGIN -->
## Bank 0 (HOME) Usage

Bank 0 is the 16 KB non-switchable ROM bank that the GB Studio engine core,
the interrupt handlers and the GBDK runtime all share. Banked ROM is cheap
(add another bank), bank 0 is not, so it is usually the first thing a project
runs out of.

| | Bytes |
|---|---|
| Bank 0 used by this plugin | **0** |
| Bank 0 free with this plugin installed | **1,451** of 16,384 (91% used) |

**This plugin costs nothing in bank 0.** All of its code lives in a switchable
ROM bank; nothing it adds is resident in bank 0.

<details><summary>How this was measured</summary>

GB Studio 4.3.2, DMG target, default engine settings. Each module's bank 0
contribution is the `A _HOME size` record that SDCC writes into its `.rel`
object, summed over the engine sources this plugin provides. Stock sizes come
from building projects whose only plugin ships no engine C, so every module in
them is the untouched engine; two such builds were compared and agreed on all
73 shared modules.

The "free" figure is a stock project with this plugin and nothing else. Your
own number will differ: other plugins, and any engine settings that change what
the core compiles, move it independently of this plugin.

</details>
<!-- BANK0:END -->

## Changelog

This plugin has not been merged into the official [gb-studio-plugins](https://github.com/gb-studio-dev/gb-studio-plugins)
repository, so the entries below are grouped by the date of the change itself.

Only bug fixes, new features and feature changes are listed. Engine version
bumps, patch regeneration, packaging fixes and documentation edits are omitted.

### 2024-08-14

- Initial release.
