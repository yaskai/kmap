#include "raylib.h"
#include "config.h"
#include "map.h"

int main() {
	Config config = (Config) { 0 };
	ConfigRead(&config, CONFIG_PATH);

	SetConfigFlags(FLAG_WINDOW_HIGHDPI);

	InitWindow(config.window_width, config.window_height, "Raylib Project");
	SetTargetFPS(config.refresh_rate);
	DisableCursor();

	Map map = (Map) { 0 };
	MapInit(&map);

	SetExitKey(KEY_F4);
	bool exit = false;

	while(!exit) {
		exit = (WindowShouldClose() || (map.flags & EXIT_REQUEST));
		
		float delta_time = GetFrameTime();

		MapUpdate(&map, delta_time);

		BeginDrawing();
		ClearBackground(BLACK);

		MapDraw(&map);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

