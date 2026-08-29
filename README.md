# gbs-reloadSceneTilesetPlugin

**Version 4.3.0. Requires GB Studio 4.3.0 or newer.**

Adds one event that reloads the current scene's background tiles from the ROM, so a scene can go
back to its original look without a scene change.

Use it after anything that has written new tile graphics into the background: Replace Tileset
Tiles, the SubmappingEx plugin, an animated tile routine of your own. On Game Boy Color both tile
banks are restored.

---

## Table of Contents

1. [Concepts](#concepts)
2. [Project Setup](#project-setup)
3. [Size Limits and Restrictions](#size-limits-and-restrictions)
4. [Events Reference](#events-reference)
5. [FAQ](#faq)
6. [Memory Footprint](#memory-footprint)
7. [Bank 0 (HOME) Usage](#bank-0-home-usage)
8. [Changelog](#changelog)

---

## Concepts

### Why a reload is needed

GB Studio uploads the scene's background tiles once, when the scene starts. Anything that later
writes new tile graphics over them keeps that change until the scene is reloaded. Leaving a room
and coming back fixes it, but that costs a scene transition.

This event repeats the upload GB Studio does at scene start, at any point you choose in a script.

### What comes back

The tile graphics only. On Game Boy Color both the monochrome tiles and the color tiles are
restored.

The background map, meaning which tile is drawn at each position, is left as it is.

---

## Project Setup

1. Copy the plugin folder into your project's `plugins` folder. There is nothing to configure.
2. Add **Reload current scene tileset** wherever the original graphics should come back. Common
   places:
   - after a run of **Replace Tileset Tiles** events you want to undo;
   - after returning from a submapped view to the scene's normal look;
   - in the **On pop** script of a SceneStackEx push, so the room looks right after a stacked
     scene returns.

The event has no fields. Placing it in the script is all it needs.

---

## Size Limits and Restrictions

### The background map is not reloaded

This event restores tile graphics. If a script has changed which tile is drawn at a given position,
using SubmappingEx or a **Set Background Tile** event, that stays as it is afterwards.

### Interface tiles are not reloaded

GB Studio reserves the last few tile slots for the dialogue font and frame. This event follows the
same layout and leaves those slots alone, as long as the tileset stays within its normal size.

### Sprites are unaffected

Sprite tiles are left alone. The one exception is a tileset with more than 256 tiles, which in
practice means only the 360-tile logo scenes. Their extra tiles go into the sprite area, matching
what GB Studio does at scene start.

### No engine files are replaced

The plugin adds a new engine file and changes none of the existing ones, so it has no conflicts
with other engine plugins.

---

## Events Reference

### Reload current scene tileset

Group: **Scene**.

Reloads every background tile for the current scene from the ROM. On Game Boy Color both tile
banks are restored. The event has no fields.

---

## FAQ

**My animated tiles will not go back to normal. Does this fix it?**
Yes. That is what it is for. One event puts every background tile back to the art the scene was
built with.

**Will it undo a Set Background Tile event?**
No. That event changes which tile is drawn where, and this one restores what the tiles look like.
To undo a background map change, write the original tile back.

**Does the screen flicker?**
The upload happens over the following screen blanks, the same way it does at scene start, so the
scene stays on screen throughout.

**Does it reset actors, variables or the camera?**
No. Only background tile graphics change. Everything else keeps running.

**Can I call it every frame?**
You can, but it is not free. Reloading a full tileset takes several frames' worth of screen blank
time. Call it once when you need the graphics back.

**Do I need it if I change scenes anyway?**
No. A scene change reloads the tiles for you. This event exists for the case where you want the
graphics back without leaving the scene.

**Does it work on Game Boy Color?**
Yes, and it restores both tile banks, so color scenes come back fully.

**Does it clash with other plugins?**
No. It adds a new engine file and replaces none of the stock ones.

---

## Memory Footprint

Measured against the stock GB Studio **4.3.0-e1** engine at default engine settings, report of
2026-08-13. Figures are the difference against a stock project. Each event you use also compiles a
few bytes of script into your project, on top of the fixed cost below.

| Budget | Cost |
|---|---|
| Bank 0 (HOME) | 0 bytes |
| WRAM | 0 bytes |
| Banked ROM | +284 bytes |

- **Bank 0:** nothing. Everything the plugin adds is compiled into a switchable ROM bank.
- **WRAM:** no change.
- **Banked ROM:** 284 bytes for the reload code.
- **Engine WRAM headroom:** a stock GB Studio 4.3.0 project leaves about **854 bytes** of WRAM
  free (the engine has 7,776 bytes to work with and uses 6,922 of them). With this plugin
  installed roughly **854 bytes** remain. Adding more global variables to your project does not
  change that figure, because script memory is a fixed 3,584 byte block at stock engine settings.
- **SRAM:** not used.

---

<!-- BANK0:BEGIN -->
## Bank 0 (HOME) Usage

Bank 0 is the 16 KB fixed ROM bank shared by the GB Studio engine core, the
interrupt handlers and the GBDK runtime. Extra banked ROM is cheap to add,
bank 0 is not, so bank 0 is usually the first thing a project runs out of.

| | Bytes |
|---|---|
| Bank 0 used by this plugin | **0** |

**This plugin costs nothing in bank 0.** Everything it adds is compiled into a
switchable ROM bank.
<!-- BANK0:END -->

## Changelog

This plugin has not been merged into the official
[gb-studio-plugins](https://github.com/gb-studio-dev/gb-studio-plugins) repository, so the entries
below are grouped by the date of the change itself.

Only bug fixes, new features and feature changes are listed. Engine version bumps, patch
regeneration, packaging fixes and documentation edits are omitted.

### 2024-08-14

- Initial release.
