#include <stdint.h>
#include "raylib.h"

#ifndef SPRITES_H_
#define SPRITES_H_

#define SPR_TEX_VALID	0x01
#define SPR_ALLOCATED	0x02
#define SPR_PERSIST  	0x04
#define SPR_FLIP_X	   	0x08
#define SPR_FLIP_Y	   	0x10

typedef struct {
	uint8_t id;

	uint8_t flags;
	uint16_t frame_count;		// Number of frames
	uint16_t cols, rows;		// Number of columns and rows

	uint16_t frame_w;			// Frame width
	uint16_t frame_h;			// Frame height
	
	Texture2D texture;			// Source image
} Spritesheet;

Spritesheet SpritesheetCreate(char *texture_path, Vector2 frame_dimensions);
Spritesheet SpritesheetCreateFromTex(Texture2D tex, Vector2 frame_dimensions);
void SpritesheetClose(Spritesheet *spritesheet);

void DrawSprite(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, uint8_t flags);
void DrawSpritePro(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, float rotation, float scale, uint8_t flags);
void DrawSpriteRecolor(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, float rotation, float scale, uint8_t flags, Color color);

uint8_t FrameIndex(Spritesheet *spritesheet, uint8_t c, uint8_t r);
Rectangle GetFrameRec(uint8_t idx, Spritesheet *spritesheet);

typedef struct {
	uint16_t frame_count;		// Total number of frames 
	uint16_t start_frame;		// Which frame is "zero" or first frame
	uint16_t cur_frame;			// Which frame should be displayed  

	uint16_t cycles;			// Number of times animation has been completed  

	float speed;				// Animation speed
	float timer;				// Timer for switching frames

	Spritesheet *spritesheet; 	// Pointer to spritesheet instance
} SpriteAnimation;

SpriteAnimation AnimCreate(Spritesheet *spritesheet, uint8_t start_frame, uint8_t frame_count, float speed);
void AnimPlay(SpriteAnimation *anim, float delta_time);
void AnimDraw(SpriteAnimation *anim, Vector2 position, uint8_t flags);
void AnimDrawPro(SpriteAnimation *anim, Vector2 position, float rotation, float scale, uint8_t flags);
void AnimReset(SpriteAnimation *anim);

#endif // !SPRITES_H_ 
