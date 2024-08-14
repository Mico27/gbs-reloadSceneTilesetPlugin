#pragma bank 255

#include <gbdk/platform.h>
#include "system.h"
#include "vm.h"
#include "gbs_types.h"
#include "bankdata.h"
#include "data_manager.h"

void reload_current_scene_tileset(SCRIPT_CTX * THIS) OLDCALL BANKED {
	scene_t scn;
    MemcpyBanked(&scn, current_scene.ptr, sizeof(scn), current_scene.bank);

	background_t bkg;
    MemcpyBanked(&bkg, scn.background.ptr, sizeof(bkg), scn.background.bank);

    load_bkg_tileset(bkg.tileset.ptr, bkg.tileset.bank);
	
	#ifdef CGB
    if ((_is_CGB) && (bkg.cgb_tileset.ptr)) {
        VBK_REG = 1;
        load_bkg_tileset(bkg.cgb_tileset.ptr, bkg.cgb_tileset.bank);
        VBK_REG = 0;
    }
#endif
}
