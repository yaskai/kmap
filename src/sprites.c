#include <stdio.h>
#include "raylib.h"
#include "sprites.h"

// Make a spritesheet 
// texture is split into rectangles based on provided dimensions
Spritesheet SpritesheetCreate(char *texture_path, Vector2 frame_dimensions) {
	// Load base texture
	Texture2D texture = LoadTexture(texture_path);
	if(!IsTextureValid(texture)) {
		printf("file missing: %s\n", texture_path);	
		return (Spritesheet){0};
	}

	// Calculate column and row count
	uint16_t cols = texture.width  / frame_dimensions.x;
	uint16_t rows = texture.height / frame_dimensions.y;
	
	// Return struct 
	return (Spritesheet) {
		.flags = (SPR_TEX_VALID),
		.frame_w = frame_dimensions.x,
		.frame_h = frame_dimensions.y,
		.cols = cols,
		.rows = rows,
		.frame_count = (cols * rows),
		.texture = texture
	};
}

Spritesheet SpritesheetCreateFromTex(Texture2D texture, Vector2 frame_dimensions) {
	// Calculate column and row count
	uint16_t cols = texture.width  / frame_dimensions.x;
	uint16_t rows = texture.height / frame_dimensions.y;
	
	// Return struct 
	return (Spritesheet) {
		.flags = (SPR_TEX_VALID),
		.frame_w = frame_dimensions.x,
		.frame_h = frame_dimensions.y,
		.cols = cols,
		.rows = rows,
		.frame_count = (cols * rows),
		.texture = texture
	};
}

// Unload data, free allocated memory
void SpritesheetClose(Spritesheet *spritesheet) {
	UnloadTexture(spritesheet->texture);
	spritesheet->flags &= ~SPR_ALLOCATED;
}

// Draw a spritesheet frame from base texture at provided position
void DrawSprite(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, uint8_t flags) {
	if(!(spritesheet->flags & SPR_TEX_VALID)) return;

	Rectangle src_rec = GetFrameRec(frame_index, spritesheet);
	if(flags & SPR_FLIP_X) src_rec.width  *= -1;
	if(flags & SPR_FLIP_Y) src_rec.height *= -1;

	DrawTextureRec(spritesheet->texture, src_rec, position, WHITE);	
}

// Draw a spritesheet frame from base texture at provided position (with rotation)
void DrawSpritePro(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, float rotation, float scale, uint8_t flags) {
	if(!(spritesheet->flags & SPR_TEX_VALID)) return;

	Rectangle src_rec = GetFrameRec(frame_index, spritesheet);

	// Flip horizontal
	if(flags & SPR_FLIP_X) src_rec.width  *= -1;

	// Flip vertical
	if(flags & SPR_FLIP_Y) src_rec.height *= -1;

	DrawTexturePro(
		spritesheet->texture,
		src_rec,

		(Rectangle) {
			position.x + (spritesheet->frame_w * 0.5f),
			position.y + (spritesheet->frame_h * 0.5f),
			spritesheet->frame_w * scale,
			spritesheet->frame_h * scale
		},

		(Vector2){(spritesheet->frame_w * 0.5f) * scale, (spritesheet->frame_h * 0.5f) * scale},
		rotation,
		WHITE	
	);
}

// Draw a spritesheet frame from base texture at provided position (with rotation)
void DrawSpriteRecolor(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, float rotation, float scale, uint8_t flags, Color color) {
	if(!(spritesheet->flags & SPR_TEX_VALID)) return;

	Rectangle src_rec = GetFrameRec(frame_index, spritesheet);

	// Flip horizontal
	if(flags & SPR_FLIP_X) src_rec.width  *= -1;

	// Flip vertical
	if(flags & SPR_FLIP_Y) src_rec.height *= -1;

	DrawTexturePro(
		spritesheet->texture,
		src_rec,

		(Rectangle) {
			position.x + (spritesheet->frame_w * 0.5f),
			position.y + (spritesheet->frame_h * 0.5f),
			spritesheet->frame_w * scale,
			spritesheet->frame_h * scale
		},

		(Vector2){(spritesheet->frame_w * 0.5f) * scale, (spritesheet->frame_h * 0.5f) * scale},
		rotation,

		color	
	);
}

// Find index of frame from it's column and row values
uint8_t FrameIndex(Spritesheet *spritesheet, uint8_t c, uint8_t r) {
	return (c + r * spritesheet->cols);
}

// Get rectangle data from frame index of spritesheet
Rectangle GetFrameRec(uint8_t idx, Spritesheet *spritesheet) {
	uint8_t c = idx % spritesheet->cols, r = idx / spritesheet->cols;

	return (Rectangle) {
		.x  = c * spritesheet->frame_w,
		.y  = r * spritesheet->frame_h,
		.width  = spritesheet->frame_w,
		.height = spritesheet->frame_h
	};			
}

// Create a new sprite animation
SpriteAnimation AnimCreate(Spritesheet *spritesheet, uint8_t start_frame, uint8_t frame_count, float speed) {
	return (SpriteAnimation) {
		.frame_count = frame_count,
		.start_frame = start_frame,
		.cur_frame = start_frame,
		.speed = (speed * 0.1f),
		.timer = (float)speed * 0.1f,
		.spritesheet = spritesheet
	};
}

// Update sprite animation
void AnimPlay(SpriteAnimation *anim, float delta_time) {
	anim->timer += delta_time;

	if(anim->timer >= anim->speed) {
		anim->cur_frame++;

		if(anim->cur_frame - anim->start_frame > anim->frame_count - 1) { 
			anim->cur_frame = anim->start_frame;
			anim->cycles++;
		}

		anim->timer = 0;
	}
}

// Draw current frame of provided sprite animation
void AnimDraw(SpriteAnimation *anim, Vector2 position, uint8_t flags) {
	DrawSprite(anim->spritesheet, anim->cur_frame, position, flags);
}

// Draw current frame of provided sprite animation (with rotation)
void AnimDrawPro(SpriteAnimation *anim, Vector2 position, float rotation, float scale, uint8_t flags) {
	DrawSpritePro(anim->spritesheet, anim->cur_frame, position, rotation, scale, flags);
}

// Reset animation state
void AnimReset(SpriteAnimation *anim) {
	anim->cur_frame = 0;
	anim->cycles = 0;
}

