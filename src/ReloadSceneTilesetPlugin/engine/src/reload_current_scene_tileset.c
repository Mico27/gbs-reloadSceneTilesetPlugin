#pragma bank 255

#include <gbdk/platform.h>
#include "system.h"
#include "vm.h"
#include "gbs_types.h"
#include "bankdata.h"
#include "data_manager.h"

static void reload_bkg_tileset(const tileset_t* tiles, UBYTE bank) {
    if ((!bank) && (!tiles)) return;

    UWORD n_tiles = ReadBankedUWORD(&(tiles->n_tiles), bank);

    // load first background chunk, align to zero tile
    UBYTE * data = tiles->tiles;
    if (n_tiles < 128) {
        if ((UBYTE)n_tiles) SetBankedBkgData(0, n_tiles, data, bank);
        return;
    }
    SetBankedBkgData(0, 128, data, bank);
    n_tiles -= 128; data += 128 * 16;

    // load second background chunk
    if (n_tiles < 128) {
        if (n_tiles < 65) {
            #ifdef ALLOC_BKG_TILES_TOWARDS_SPR
                // new allocation style, align to 192-th tile
                if ((UBYTE)n_tiles) SetBankedBkgData(192 - n_tiles, n_tiles, data, bank);
            #else
                // old allocation style, align to 128-th tile
                if ((UBYTE)n_tiles) SetBankedBkgData(128, n_tiles, data, bank);
            #endif
        } else {
            // if greater than 64 allow overflow into UI, align to 128-th tile
            if ((UBYTE)n_tiles) SetBankedBkgData(128, n_tiles, data, bank);
        }
        return;
    }
    SetBankedBkgData(128, 128, data, bank);
    n_tiles -= 128; data += 128 * 16;

    // if more than 256 - then it's a 360-tile logo, load rest to sprite area
    if ((UBYTE)n_tiles) SetBankedSpriteData(0, n_tiles, data, bank);
}

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
