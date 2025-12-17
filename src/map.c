#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "map.h"

// Pitch, yaw, roll for camera
float cam_p, cam_y, cam_r;

void MapInit(Map *map) {
	map->camera = (Camera3D) {
		.position = (Vector3) { 0, 0, 0 },
		.target = (Vector3) { 1, 0, 0 },
		.up = CAMERA_UP,
		.fovy = 120,
		.projection = CAMERA_PERSPECTIVE
	};

	GridInit(&map->grid, (Coords) { 16, 4, 16 }, 4);

	Grid *grid = &map->grid;
}

void MapUpdate(Map *map, float dt) {
	if(IsKeyPressed(KEY_ESCAPE))
		map->edit_mode = !map->edit_mode;

	if(map->edit_mode == MODE_NORMAL) 
		CameraControls(map, dt);

	UpdateDrawList(map, &map->grid);
}

void MapDraw(Map *map) {
	BeginMode3D(map->camera);
	
	uint8_t draw_cells_flags = (DCELLS_DRAW_BOXES | DCELLS_OCCLUSION | DCELLS_ONLY_FLOOR);
	DrawCells(map, &map->grid, draw_cells_flags);

	EndMode3D();

	char *mode_text = (!map->edit_mode) ? "normal" : "insert";	
	DrawText(TextFormat("mode: %s", mode_text), 0, 1080 - 20, 20, RAYWHITE);
}

void GenerateAssetTable(Map *map, char *path) {
}

void GridInit(Grid *grid, Coords dimensions, float cell_size) {
	// Free existing data
	if(grid->data) 
		free(grid->data);

	if(grid->draw_list) 
		free(grid->draw_list);

	// Create new grid
	Grid new_grid = (Grid) {
		.draw_list = NULL,
		.data = NULL,

		.cell_size = cell_size,

		.cell_count = (dimensions.c * dimensions.r * dimensions.t),

		.cols = dimensions.c,	
		.rows = dimensions.r,	
		.tabs = dimensions.t,	
	};

	// Allocate memory
	new_grid.draw_list = calloc(new_grid.cell_count, sizeof(int32_t));
	new_grid.data = calloc(new_grid.cell_count, sizeof(unsigned char));

	new_grid.data[0] = 'x';

	// Overwrite grid with new values
	*grid = new_grid;
}

int16_t CellCoordsToId(Coords coords, Grid *grid) {
	return (int16_t)(coords.r + coords.c * grid->cols + coords.t * grid->tabs);
}

Coords CellIdToCoords(int16_t id, Grid *grid) {
	return (Coords) {
		.c = id % grid->cols,					// x,
		.r = (id / grid->cols) % grid->rows,	// y,
		.t = id / (grid->cols * grid->rows)		// z
	};
}

Coords Vec3ToCoords(Vector3 v, Grid *grid) {
	return (Coords) {
		.c = roundf(v.x) / grid->cell_size,
		.r = roundf(v.y) / grid->cell_size,
		.t = roundf(v.z) / grid->cell_size
	};
}

Vector3 CoordsToVec3(Coords coords, Grid *grid) {
	return Vector3Scale((Vector3) { coords.c, coords.r, coords.t }, grid->cell_size);
}

void UpdateDrawList(Map *map, Grid *grid) {
	grid->draw_count = 0;

	for(int32_t i = 0; i < grid->cell_count; i++) {
		Coords coords = CellIdToCoords(i, grid);

		Vector3 position = (Vector3) { coords.c, coords.r, coords.t }; 
		position = Vector3Scale(position, grid->cell_size);

		Vector3 camera_forward = Vector3Subtract(map->camera.target, map->camera.position);
		Vector3 camera_to_cell = Vector3Subtract(position, map->camera.position);
		
		float dot = Vector3DotProduct(Vector3Normalize(camera_forward), Vector3Normalize(camera_to_cell));
		if(dot < 0.0f) continue;

		if(Vector3Length(camera_to_cell) > 24 * grid->cell_size) continue;

		grid->draw_list[grid->draw_count++] = i;
	}
}

void DrawCells(Map *map, Grid *grid, uint8_t flags) {
	Vector3 cell_size_v = Vector3Scale(Vector3One(), grid->cell_size);

	for(int32_t i = 0; i < grid->draw_count; i++) {
		// Get id of cell to draw
		int32_t cell_id = grid->draw_list[i];

		// Get cell's coordinates
		Coords coords = CellIdToCoords(grid->draw_list[i], grid);

		// Calculate world space position
		Vector3 position = CoordsToVec3(coords, grid);

		float dist = Vector3Distance(position, map->camera.position);
		//float d = fmaxf(map->camera.position.y, position.y) - fminf(map->camera.position.y, position.y);			
		float d = Vector3Length(Vector3Subtract(map->camera.position, position)) * 0.65f;	
		d = Clamp(d, 0.0f, 0.75f);

		Color color = ColorAlpha(GRAY, 1 - d);

		if(flags & DCELLS_DRAW_BOXES) {
			if(flags & DCELLS_ONLY_FLOOR) {

				if(coords.r != 0) continue;
					
				DrawCubeWiresV ( (Vector3) { position.x, position.y - grid->cell_size * 0.51f, position.z },
								 (Vector3) { cell_size_v.x, 0.1f, cell_size_v.z },
								 color
								 );
			} else
				DrawCubeWiresV(position, cell_size_v, color);
		}

		if(!grid->data[cell_id])
			continue;

		switch(grid->data[cell_id]) {
			case 'x':
				DrawCubeV(position, cell_size_v, ColorAlpha(MAGENTA, 0.5f));
				break;
		}
	}	

	DrawCubeWiresV(
		Vector3Scale((Vector3){grid->cols - 1, grid->rows - 1, grid->tabs - 1}, grid->cell_size * 0.5f),
		Vector3Scale((Vector3){grid->cols, grid->rows, grid->tabs}, grid->cell_size),
		LIGHTGRAY
	);

	/*
	DrawCubeV(
		Vector3Scale((Vector3){grid->cols - 1, grid->rows - 1, grid->tabs - 1}, grid->cell_size * 0.5f),
		Vector3Scale((Vector3){grid->cols, grid->rows, grid->tabs}, grid->cell_size),
		ColorAlpha(RED, 0.5f)
	);
	*/
}

// Use keyboard and mouse input to move camera
void CameraControls(Map *map, float dt) {
	Camera *cam = &map->camera;

	Vector2 mouse_delta = GetMouseDelta();
	cam_p -= mouse_delta.y * CAMERA_SENSITIVITY * dt;
	cam_y += mouse_delta.x * CAMERA_SENSITIVITY * dt;

	Vector3 forward = (Vector3) { .x = cosf(cam_y) * cosf(cam_p), .y = sinf(cam_p), .z = sinf(cam_y) * cosf(cam_p) };
	forward = Vector3Normalize(forward);

	Vector3 right = Vector3CrossProduct(forward, CAMERA_UP);

	cam->target = Vector3Add(cam->position, Vector3Scale(forward, 10));

	Vector3 movement = Vector3Zero();

	// Pan with WASD:
	// Move forward
	if(IsKeyDown(KEY_W)) 
		movement = Vector3Add(movement, forward);

	// Move left
	if(IsKeyDown(KEY_A))
		movement = Vector3Subtract(movement, right);

	// Move backwards
	if(IsKeyDown(KEY_S))
		movement = Vector3Subtract(movement, forward);

	// Move right
	if(IsKeyDown(KEY_D))
		movement = Vector3Add(movement, right);
	
	movement = Vector3Scale(movement, CAMERA_SPEED * dt);
	cam->position = Vector3Add(cam->position, movement);
	cam->target = Vector3Add(cam->target, movement);
}

