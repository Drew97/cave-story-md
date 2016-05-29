#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <types.h>

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define SCREEN_HALF_W 160
#define SCREEN_HALF_H 112

// Direction
enum { DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_CENTER };

// Tileset width/height
// The only map larger than this is hell (have to figure out something for that)
#define TS_WIDTH 32
#define TS_HEIGHT 20

// Stage tileset is first in USERINDEX
#define TILE_TSINDEX TILE_USERINDEX
#define TILE_TSSIZE (TS_WIDTH * TS_HEIGHT)
#define TILE_FACEINDEX (TILE_TSINDEX + TILE_TSSIZE)
#define TILE_FACESIZE 36
// VRAM reserved for SGDK sprite engine
#define TILE_SPRITEINDEX (TILE_FACEINDEX + TILE_FACESIZE)
#define TILE_SPRITESIZE (TILE_MAXNUM - TILE_SPRITEINDEX)
// Extra space for tiles between planes
#define TILE_EXTRA1INDEX (0xD000 >> 5)
#define TILE_EXTRA2INDEX (0xF000 >> 5)
// Allocation of EXTRA1 (128 tiles)
#define TILE_BACKINDEX TILE_EXTRA1INDEX
#define BACK_SIZE 96
#define TILE_HUDINDEX (TILE_BACKINDEX + BACK_SIZE)
#define HUD_SIZE 32
// Allocation of EXTRA2 (64 tiles)
#define TILE_NUMBERINDEX TILE_EXTRA2INDEX
#define TILE_NUMBERSIZE 16
#define TILE_SMOKEINDEX (TILE_NUMBERINDEX + TILE_NUMBERSIZE)
#define TILE_SMOKESIZE 28
#define TILE_WINDOWINDEX (TILE_SMOKEINDEX + TILE_SMOKESIZE)
#define TILE_WINDOWSIZE 18

// Unit conversions
// sub - fixed point unit (1/512x1/512)
// pixel - single dot on screen (1x1)
// tile - genesis VDP tile (8x8)
// block - Cave Story tile (16x16)
#define sub_to_pixel(x)   ((x)>>9)
#define sub_to_tile(x)    ((x)>>12)
#define sub_to_block(x)   ((x)>>13)

#define pixel_to_sub(x)   ((x)<<9)
#define pixel_to_tile(x)  ((x)>>3)
#define pixel_to_block(x) ((x)>>4)

#define tile_to_sub(x)    ((x)<<12)
#define tile_to_pixel(x)  ((x)<<3)
#define tile_to_block(x)  ((x)>>1)

#define block_to_sub(x)   ((x)<<13)
#define block_to_pixel(x) ((x)<<4)
#define block_to_tile(x)  ((x)<<1)

#define floor(x) ((x)&~0x1FF)
#define round(x) (((x)+0x100)&~0x1FF)
#define ceil(x)  (((x)+0x1FF)&~0x1FF)

// Get tileset from SpriteDefinition
#define SPR_TILESET(spr, a, f) (spr.animations[a]->frames[f]->tileset)

#define SPR_SAFERELEASE(s); if(s != NULL) { SPR_releaseSprite(s); s = NULL; }

// Booleans
typedef unsigned char bool;
enum {false, true};

// Generic function pointer
typedef void (*func)();

// Bounding box
typedef struct {
	u8 left;
	u8 top;
	u8 right;
	u8 bottom;
} bounding_box;

#endif // INC_COMMON_H_
