#include <stdint.h>
#include "raylib.h"

#ifndef WATER_H_
#define WATER_H_

#define PX_COUNT (uint32_t)(512 * 512)

typedef struct {
	Texture2D output;

	Color *output_px;
	uint8_t *noise;

	float timer;
	float filter;

	uint16_t scroll_x, scroll_y;
	uint16_t offset;

} WaterBackground;

void WaterInit(WaterBackground *bg);
void WaterUpdate(WaterBackground *bg, float dt);
void WaterDraw(WaterBackground *bg, int ww, int wh);
void WaterDrawTile(WaterBackground *bg, int x, int y);
void WaterClose(WaterBackground *bg);

#endif
