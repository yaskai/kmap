#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "water.h"

void WaterInit(WaterBackground *bg) {
	bg->scroll_x = 0, bg->scroll_y = 0;
	bg->offset = 213;
	bg->timer = 0.0f;
	bg->filter = 0.86f;

	bg->noise = (uint8_t*)malloc(PX_COUNT);
	bg->output_px = (Color*)malloc(sizeof(Color) * PX_COUNT);

	Image img = LoadImage("resources/water_noise.png");
	Color *px = LoadImageColors(img);

	for(uint32_t i = 0; i < PX_COUNT; i++) {
		bg->noise[i] = (uint8_t)px[i].r;
		bg->output_px[i] = px[i];
	}

	bg->output = LoadTextureFromImage(img);
	SetTextureWrap(bg->output, TEXTURE_WRAP_REPEAT);
	GenTextureMipmaps(&bg->output);
	SetTextureFilter(bg->output, TEXTURE_FILTER_TRILINEAR);

	UnloadImage(img);
	free(px);
}

void WaterUpdate(WaterBackground *bg, float dt) {
	if(IsKeyDown(KEY_UP)) bg->filter += 0.001f;
	if(IsKeyDown(KEY_DOWN)) bg->filter -= 0.001f;
	bg->filter = Clamp(bg->filter, 0, 1);

	if(bg->timer > 0) {
		bg->timer -= 10 * dt;
		return;
	}

	bg->scroll_x = (bg->scroll_x + 1) % PX_COUNT;
	bg->scroll_y = (bg->scroll_y + 1) % PX_COUNT;

	for(uint32_t i = 0; i < PX_COUNT; i++) {
		uint32_t x = ((i % 512) + bg->scroll_x) % 512; 
		uint32_t y = ((i / 512) + bg->scroll_y) % 512;
		uint32_t idxA = (x + y * 512);

		uint32_t offx = (((i % 512) + bg->offset) - bg->scroll_x) % 512;
		uint32_t offy = (((i / 512) - bg->offset) + bg->scroll_y) % 512;
		uint32_t idxB = (offx + offy * 512);

		float val = Clamp(((bg->noise[idxA] + bg->noise[idxB]) * bg->filter), 0, 255);
		Color processed = (Color){10, 70, 255, 230};

		if(val > 240 && val < 254) {
			processed = (Color) {
				.r = 220,
				.g = 240,
				.b = 245,
				.a = 200 
			};
		} else { 
			processed.r *= 0.25f;
			processed.g *= 0.25f;
			processed.b *= 0.75f;
			processed.a = 20;
		}

		if(val < 225 && val > 70) {
			processed.r *= 0.5f;
			processed.g *= 0.5f;
			processed.b *= 2.0f;
			processed.a = 10;
		}

		if(processed.a > 210) processed.a = 200;
		else processed.a = 100;
		
		bg->output_px[i] = processed;
	}

	UpdateTexture(bg->output, bg->output_px);
	bg->timer = 0.0175f;
}

void WaterDraw(WaterBackground *bg, int ww, int wh) {
	DrawTexturePro(bg->output, (Rectangle){0, 0, ww, wh}, (Rectangle){0, 0, ww, wh}, Vector2Zero(), 0, WHITE);
	DrawRectangleV(Vector2Zero(), (Vector2){ww, 40}, ColorAlpha(BLACK, 0.25f));
	DrawText(TextFormat("%.3f", bg->filter), 0, 0, 40, RAYWHITE);
}

void WaterClose(WaterBackground *bg) {
	UnloadTexture(bg->output);	
	free(bg->output_px);
	free(bg->noise);
}

