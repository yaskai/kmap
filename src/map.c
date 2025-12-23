#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "lights.h"
#include "raylib.h"
#include "raymath.h"
#include "map.h"

// Pitch, yaw, roll for camera
float cam_p, cam_y, cam_r;

Coords hover_coords;
Ray debug_ray;

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

	GuiInit(&map->gui);

	InitLights(&map->light_handler);

	MakeLight(0, 100, CoordsToVec3( (Coords) { grid->cols / 2, grid->rows, grid->tabs / 2 }, &map->grid), WHITE, &map->light_handler);

	//MakeLight(0, 700, CoordsToVec3( (Coords) { 0, grid->rows / 2, 0 }, &map->grid), WHITE, &map->light_handler);
	//MakeLight(0, 700, CoordsToVec3( (Coords) { grid->cols / 2 , grid->rows / 2, grid->tabs }, &map->grid), WHITE, &map->light_handler);
	//MakeLight(0, 700, CoordsToVec3( (Coords) { 0, grid->rows / 2, grid->tabs / 2 }, &map->grid), WHITE, &map->light_handler);

	GenerateAssetTable(map, "");

	map->action_count = 0;
	map->action_cap = 1;

	map->actions_undo = malloc(sizeof(Action));
	map->actions_redo = malloc(sizeof(Action));
}

void MapUpdate(Map *map, float dt) {
	UpdateLights(&map->light_handler);

	// Set which tiles to render
	UpdateDrawList(map, &map->grid);

	switch(map->edit_mode) {
		case MODE_NORMAL:
			MapUpdateModeNormal(map, dt);
			break;

		case MODE_INSERT:
			MapUpdateModeInsert(map, dt);
			break;
	}

	// Toggle edit mode
	if(IsKeyPressed(KEY_ESCAPE)) {
		map->edit_mode = !map->edit_mode;

		switch(map->edit_mode) {
			case MODE_NORMAL:
				SetMousePosition(0, 0);
				DisableCursor();
				break;

			case MODE_INSERT:
				EnableCursor();
				HideCursor();
				break;
		}
	}
}

void MapDraw(Map *map) {
	BeginMode3D(map->camera);
	
	uint8_t draw_cells_flags = (DCELLS_DRAW_BOXES | DCELLS_OCCLUSION | DCELLS_ONLY_FLOOR);
	DrawCells(map, &map->grid, draw_cells_flags);

	/*
	for(uint8_t i = 0; i < map->light_handler.light_count; i++)
		DrawLightGizmos(&map->light_handler, i);
	*/

	EndMode3D();

	char *mode_text = (!map->edit_mode) ? "normal" : "insert";	
	DrawText(TextFormat("mode: %s", mode_text), 0, 1080 - 20, 20, RAYWHITE);

	DrawText(TextFormat("action count: %d", map->action_count), 0, 0, 30, RAYWHITE);
	DrawText(TextFormat("current action: %d", map->curr_action), 0, 30, 30, RAYWHITE);

	if(map->edit_mode == MODE_INSERT) 
		GuiUpdate(&map->gui);
}

// Update loop for normal mode
void MapUpdateModeNormal(Map *map, float dt) {
	// Move camera with input
	CameraControls(map, dt);
}

// Update loop for insert mode
void MapUpdateModeInsert(Map *map, float dt) {
	Vector2 cursor_pos_screen = GetMousePosition();
	//Ray ray = GetScreenToWorldRayEx(cursor_pos_screen, map->camera, 1920, 1080);
	Ray ray = GetScreenToWorldRay(cursor_pos_screen, map->camera);

	debug_ray = ray;

	Vector3 sphere_pos = Vector3Add(debug_ray.position, Vector3Scale(debug_ray.direction, 10));
	Coords cursor_coords = Vec3ToCoords(sphere_pos, &map->grid);

	if(CoordsInBounds(cursor_coords, &map->grid)) {
		hover_coords = cursor_coords;
	}

	if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Action action_add_block = (Action) {
			.cells = malloc(sizeof(uint32_t)),
			.data = malloc(sizeof(unsigned char)),
			.cell_count = 1,
			.id = map->action_count
		};

		action_add_block.cells[0] = CellCoordsToId(hover_coords, &map->grid);
		action_add_block.data[0] = 'x';
		
		ActionApply(&action_add_block, map);
	} 

	if(IsKeyPressed(KEY_Z)) {
		ActionUndo(map);
	}

	if(IsKeyPressed(KEY_R)) {
		ActionRedo(map);
	}
}

void GenerateAssetTable(Map *map, char *path) {
	map->asset_table = malloc(sizeof(Asset) * 4);	

	Mesh base_mesh = GenMeshCube(map->grid.cell_size, map->grid.cell_size, map->grid.cell_size);
	Texture2D base_tex = LoadTexture("resources/base_tex.png");

	map->asset_table[0] = (Asset) {
		.model = LoadModelFromMesh(base_mesh),
	};

	for(int i = 0; i < map->asset_table[0].model.materialCount; i++) {
		map->asset_table[0].model.materials[i].maps->texture = base_tex;
		map->asset_table[0].model.materials[i].shader = map->light_handler.shader;
	}
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

	// Overwrite grid with new values
	*grid = new_grid;
}

int16_t CellCoordsToId(Coords coords, Grid *grid) {
	return (coords.c + coords.r * grid->cols + coords.t * grid->cols * grid->rows);
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

bool CoordsInBounds(Coords coords, Grid *grid) {
	return ( coords.c > -1 && coords.c < grid->cols -1 &&
			 coords.r > -1 && coords.r < grid->rows -1 &&
			 coords.t > -1 && coords.t < grid->tabs -1 );
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
		float d = Vector3Length(Vector3Subtract(map->camera.position, position)) * 0.65f;	
		d = Clamp(d, 0.0f, 0.75f);

		Color color = ColorAlpha(GRAY, 1 - d);

		if(flags & DCELLS_DRAW_BOXES) {
			/*
			if(flags & DCELLS_ONLY_FLOOR && grid->data[cell_id] == 0) {

				if(coords.r != 0) continue;
					
				DrawCubeWiresV ( (Vector3) { position.x, position.y - grid->cell_size * 0.51f, position.z },
								 (Vector3) { cell_size_v.x, 0.1f, cell_size_v.z },
								 color
								 );
			} else
				DrawCubeWiresV(position, cell_size_v, color);
			*/
		}

		if(!grid->data[cell_id])
			continue;

		switch(grid->data[cell_id]) {
			case 'x':
				//DrawCubeV(position, cell_size_v, ColorAlpha(MAGENTA, 0.5f));
				//DrawCubeV(position, cell_size_v, ColorAlpha(MAGENTA, 1.0f));
				//DrawModel(map->asset_table[0].model, position, 1, WHITE);
				DrawModelShaded(map->asset_table[0].model, position);
				//DrawModelEx(map->asset_table[0].model, position, CAMERA_UP, 0, Vector3One(), WHITE);
				break;
		}
	}	

	DrawCubeWiresV(CoordsToVec3(hover_coords, grid), cell_size_v, BLUE);

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

	//DrawRay(debug_ray, RED);
	//Vector3 sphere_pos = Vector3Add(debug_ray.position, Vector3Scale(debug_ray.direction, 10));
	//DrawSphere(sphere_pos, 1, ColorAlpha(RED, 0.5f));
}

// Use keyboard and mouse input to move camera
void CameraControls(Map *map, float dt) {
	Camera *cam = &map->camera;

	Vector2 mouse_delta = GetMouseDelta();
	cam_p -= mouse_delta.y * CAMERA_SENSITIVITY * dt;
	cam_y += mouse_delta.x * CAMERA_SENSITIVITY * dt;
	cam_p = Clamp(cam_p, -CAMERA_MAX_PITCH, CAMERA_MAX_PITCH);

	Vector3 forward = (Vector3) { .x = cosf(cam_y) * cosf(cam_p), .y = sinf(cam_p), .z = sinf(cam_y) * cosf(cam_p) };
	forward = Vector3Normalize(forward);

	Vector3 right = Vector3CrossProduct(forward, CAMERA_UP);

	cam->target = Vector3Add(cam->position, Vector3Scale(forward, 10));

	Vector3 movement = Vector3Zero();

	// Pan with WASD:
	// Move forward
	if(IsKeyDown(KEY_W)) movement = Vector3Add(movement, forward);
	// Move left
	if(IsKeyDown(KEY_A)) movement = Vector3Subtract(movement, right);
	// Move backwards
	if(IsKeyDown(KEY_S)) movement = Vector3Subtract(movement, forward);
	// Move right
	if(IsKeyDown(KEY_D)) movement = Vector3Add(movement, right);
	
	movement = Vector3Scale(movement, CAMERA_SPEED * dt);
	cam->position = Vector3Add(cam->position, movement);
	cam->target = Vector3Add(cam->target, movement);
}

void ActionApply(Action *action, Map *map) {
	if(map->action_count + 1 > map->action_cap - 1) {
		map->action_cap *= 2;

		Action *new_undo_ptr = realloc(map->actions_undo, sizeof(Action) * map->action_cap);
		Action *new_redo_ptr = realloc(map->actions_redo, sizeof(Action) * map->action_cap);

		map->actions_undo = new_undo_ptr;
		map->actions_redo = new_redo_ptr;
	}

	if(map->curr_action < map->action_count) {
		for(uint16_t i = map->curr_action; i < map->action_count; i++) {
			ActionFreeData(&map->actions_undo[i]);
			ActionFreeData(&map->actions_redo[i]);
		}

		map->action_count = map->curr_action;
	}

	Action redo_action = (Action) {
		.cells = malloc(sizeof(uint32_t) * action->cell_count), 
		.data = malloc(sizeof(unsigned char) * action->cell_count),
		.cell_count = action->cell_count,
		.id = action->id
	};

	Action undo_action = (Action) {
		.cells = malloc(sizeof(uint32_t) * action->cell_count), 
		.data = malloc(sizeof(unsigned char) * action->cell_count),
		.cell_count = action->cell_count,
		.id = action->id
	};

	for(uint32_t i = 0; i < action->cell_count; i++) {
		uint32_t cell_id = action->cells[i];	

		undo_action.cells[i] = cell_id;
		redo_action.cells[i] = cell_id;

		undo_action.data[i] = map->grid.data[cell_id]; 
		redo_action.data[i] = action->data[i];

		map->grid.data[cell_id] = action->data[i];
	}

	map->actions_redo[map->action_count] = redo_action; 
	map->actions_undo[map->action_count] = undo_action; 

	map->action_count++;
	map->curr_action = map->action_count;
}

void ActionUndo(Map *map) {
	if(map->curr_action < 1) return; 

	map->curr_action--;

	Action *action_undo = &map->actions_undo[map->curr_action];

	for(uint32_t i = 0; i < action_undo->cell_count; i++) {
		uint32_t cell_id = action_undo->cells[i];
		map->grid.data[cell_id] = action_undo->data[i];
	}
}

void ActionRedo(Map *map) {
	if(map->curr_action > map->action_count - 1) return;

	Action *action_redo = &map->actions_redo[map->curr_action];

	for(uint32_t i = 0; i < action_redo->cell_count; i++) {
		uint32_t cell_id = action_redo->cells[i];
		map->grid.data[cell_id] = action_redo->data[i];
	}

	map->curr_action++;
}

void ActionFreeData(Action *action) {
	if(action->data)
		free(action->data);

	if(action->cells)
		free(action->cells);

	*action = (Action) { 0 };
}
