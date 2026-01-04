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
	//GenTextureMipmaps(&bg->output);
	//SetTextureFilter(bg->output, TEXTURE_FILTER_TRILINEAR);

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

	bg->scroll_x = (bg->scroll_x + 1) % 512;
	bg->scroll_y = (bg->scroll_y + 1) % 512;

	for(uint32_t i = 0; i < PX_COUNT; i++) {
		uint32_t x = ((i % 512) + bg->scroll_x) % 512; 
		uint32_t y = ((i / 512) + bg->scroll_y) % 512;
		uint32_t idxA = (x + y * 512);

		//uint32_t offx = (bg->scroll_x - (((i % 512) - bg->offset) - bg->scroll_x)) % 512;
		//uint32_t offy = (bg->scroll_y - (((i / 512) - bg->offset) - bg->scroll_y)) % 512;
		uint32_t offx = ((i % 512) - bg->scroll_x + bg->offset) % 512;
		uint32_t offy = ((i / 512) - bg->scroll_y - bg->offset) % 512;
		uint32_t idxB = (offx + offy * 512);

		//float val = Clamp(((bg->noise[idxA] + bg->noise[idxB]) * bg->filter), 0, 255);
		float val = Clamp(((bg->noise[idxA] + bg->noise[idxB]) * bg->filter), 0, 255);
		Color processed = (Color){val, val, val, val};
		//Color processed = (Color){0, 100, 255, 0};

		float intensity = val / 255.0f;

		//if(intensity >= bg->filter) processed = (Color) { 0, 0, 0, 0 };
		//if(intensity > 0.85f) processed = (Color) { 255, 255, 255, 150 };
		
		if(intensity > 0.79f) processed = (Color) { 0, 0, 0, 100 };
		if(intensity > 0.99f) processed = (Color) { 240, 240, 250, 100};
		
		//if(intensity >= 0.95f) processed.a = 250;

		bg->output_px[i] = processed;
	}

	UpdateTexture(bg->output, bg->output_px);
	bg->timer = 0.0175f;
}

void WaterDraw(WaterBackground *bg, int ww, int wh) {
	DrawTexturePro(bg->output, (Rectangle){0, 0, ww, wh}, (Rectangle){0, 0, ww, wh}, Vector2Zero(), 0, WHITE);
	//DrawRectangleV(Vector2Zero(), (Vector2){ww, 40}, ColorAlpha(BLACK, 0.25f));
	//DrawText(TextFormat("%.3f", bg->filter), 0, 0, 40, RAYWHITE);
}

void WaterDrawTile(WaterBackground *bg, int x, int y) {
	int w = 512 * 0.5f;
	int h = 512 * 0.5f;

	//DrawTextureRec(bg->output, (Rectangle) { x * w, y * h, w, h }, Vector2Zero(), WHITE);
	//DrawTexturePro(bg->output, (Rectangle){x * w, y * h, w, h}, (Rectangle){0, 0, w, -h}, Vector2Zero(), 0, WHITE);
	DrawTexturePro(bg->output, (Rectangle){x * w, y * h, w, h}, (Rectangle){0, 0, w, -h}, Vector2Zero(), 0, WHITE);
}

void WaterClose(WaterBackground *bg) {
	UnloadTexture(bg->output);	
	free(bg->output_px);
	free(bg->noise);
}

