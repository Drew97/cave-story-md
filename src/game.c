#include "game.h"

#include <genesis.h>
#include "resources.h"
#include "input.h"
#include "system.h"
#include "player.h"
#include "stage.h"
#include "camera.h"
#include "tables.h"
#include "tsc.h"
#include "vdp_ext.h"
#include "effect.h"
#include "hud.h"

//u8 debugTime = 1;
//u32 playerProf, entityProf;

bool update_pause() {
	if(joy_pressed(BUTTON_START)) {
		player_unpause();
		return false;
	} else {
		
		// TODO: Item Menu
	}
	return true;
}

void game_reset(bool load) {
	camera_init();
	tsc_init();
	if(load) {
		system_load();
	} else {
		system_new();
		tsc_call_event(GAME_START_EVENT);
	}
	// Load up the main palette
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	VDP_setPalette(PAL3, PAL_Regu.data);
}

void vblank() {
	stage_update();
	if(hudRedrawPending) hud_update_vblank();
	/* TODO: Rewrite with sprintf
	if(debuggingEnabled && --debugTime == 0) {
		char str[34];
		// Draw player/entity update time
		uintToStr(playerProf, &str[0], 5);
		uintToStr(entityProf, &str[6], 5);
		// Entity count
		uintToStr(entities_count_active(), &str[15], 3);
		uintToStr(entities_count(), &str[19], 3);
		// Free Memory
		uintToStr(MEM_getFree(), &str[27], 5);
		str[5] = str[11] = str[22] = ' ';
		str[12] = 'E'; str[13] = '#'; str[14] = ':'; str[18] = '/';
		str[23] = str[25] = 'M'; str[24] = 'E'; str[26] = ':';
		VDP_drawTextWindow(str, 1, 27);
		debugTime = 60;
	}
	*/
}

u8 game_main(bool load) {
	// If player chooses continue with no save, start a new game
	if(load && !system_checkdata()) {
		load = false;
	}
	SYS_disableInts();
	VDP_setEnable(false);
	
	//SPR_reset();
	VDP_resetScreen();
	// This is the SGDK font with a blue background for the message window
	VDP_loadTileSet(&TS_MsgFont, TILE_FONTINDEX, true);
	SYS_setVIntCallback(vblank);
	// A couple backgrounds (clouds) use tile scrolling
	VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
	effects_init();
	game_reset(load);
	
	VDP_setEnable(true);
	VDP_setWindowPos(0, 0);
	SYS_enableInts();
	
	bool paused = false;
	u8 ending = 0;
	
	while(true) {
		input_update();
		if(paused) {
			paused = update_pause();
		} else {
			if(!tsc_running() && joy_pressed(BUTTON_START)) {
				//draw_pause();
				paused = true;
			} else {
				//if(debuggingEnabled) debug_update();
				camera_update();
				//playerProf = getSubTick();
				player_update();
				//playerProf = getSubTick() - playerProf;
				hud_update();
				//entityProf = getSubTick();
				entities_update();
				//entityProf = getSubTick() - entityProf;
				u8 rtn = tsc_update();
				if(rtn > 0) {
					if(rtn == 1) {
						ending = 0; // No ending, return to title
						break;
					} else if(rtn == 2) {
						game_reset(true); // Reload save
						continue;
					} else if(rtn == 3) {
						game_reset(false); // Start from beginning
						continue;
					} else if(rtn == 4) {
						ending = 1; // Normal ending
						break;
					} else if(rtn == 5) {
						ending = 2; // Good ending
						break;
					}
				}
				effects_update();
				system_update();
			}
		}
		SPR_update();
		VDP_waitVSync();
	}
	SYS_setVIntCallback(NULL);
	return ending;
}
