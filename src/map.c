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

	MakeLight(0, 500, CoordsToVec3( (Coords) { grid->cols / 2, grid->rows - 2, grid->tabs / 2 }, &map->grid), WHITE, &map->light_handler);

	//MakeLight(0, 700, CoordsToVec3( (Coords) { 0, grid->rows / 2, 0 }, &map->grid), WHITE, &map->light_handler);
	//MakeLight(0, 700, CoordsToVec3( (Coords) { grid->cols / 2 , grid->rows / 2, grid->tabs }, &map->grid), WHITE, &map->light_handler);
	//MakeLight(0, 700, CoordsToVec3( (Coords) { 0, grid->rows / 2, grid->tabs / 2 }, &map->grid), WHITE, &map->light_handler);

	GenerateAssetTable(map, "");

	map->action_count = 0;
	map->action_cap = 1;

	map->actions_undo = malloc(sizeof(Action));
	map->actions_redo = malloc(sizeof(Action));

	map->block_selected = 'x';
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

	for(uint8_t i = 0; i < map->light_handler.light_count; i++)
		DrawLightGizmos(&map->light_handler, i);

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
			.rotation = calloc(1, sizeof(uint8_t)),
			.cell_count = 1,
			.id = map->action_count
		};

		action_add_block.cells[0] = CellCoordsToId(hover_coords, &map->grid);
		action_add_block.data[0] = map->block_selected;
		action_add_block.rotation[0] = 0;
		
		ActionApply(&action_add_block, map);
	} 

	if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
		Action action_remove_block = (Action) {
			.cells = malloc(sizeof(uint32_t)),
			.data = malloc(sizeof(unsigned char)),
			.rotation = malloc(sizeof(uint8_t)),
			.cell_count = 1,
			.id = map->action_count
		};

		action_remove_block.cells[0] = CellCoordsToId(hover_coords, &map->grid);
		action_remove_block.data[0] = 0;
		action_remove_block.rotation[0] = 0;
		
		ActionApply(&action_remove_block, map);
	} 

	if(IsKeyPressed(KEY_Z)) ActionUndo(map);
	if(IsKeyPressed(KEY_R)) ActionRedo(map);

	if(IsKeyPressed(KEY_E)) { 
		MapExportLayout(map, "test.lvl");
	}

	if(IsKeyPressed(KEY_I)) {
	}

	uint32_t hover_id = CellCoordsToId(hover_coords, &map->grid);

	if(IsKeyPressed(KEY_R)) {
		if(map->grid.data[hover_id]) {

			Action action_rotate_block = (Action) {
				.cells = malloc(sizeof(uint32_t)),
				.data = malloc(sizeof(unsigned char)),
				.rotation = malloc(sizeof(uint8_t)),
				.cell_count = 1,
				.id = map->action_count
			};

			action_rotate_block.cells[0] = CellCoordsToId(hover_coords, &map->grid);
			action_rotate_block.data[0] = map->grid.data[hover_id];
			action_rotate_block.rotation[0] = (map->grid.rotation[hover_id] + 1);

			if(action_rotate_block.rotation[0] > 3)
				action_rotate_block.rotation[0] = 0;
		
			ActionApply(&action_rotate_block, map);
		}
	}

	if(IsKeyPressed(KEY_ONE)) 
		map->block_selected = 'x';

	if(IsKeyPressed(KEY_TWO)) 
		map->block_selected = 'c';
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

	map->asset_table[1] = (Asset) {
		.model = LoadModel("resources/corner.obj"),
	};

	for(int i = 0; i < map->asset_table[1].model.materialCount; i++) {
		map->asset_table[1].model.materials[i].maps->texture = base_tex;
		map->asset_table[1].model.materials[i].shader = map->light_handler.shader;
	}
}

void GridInit(Grid *grid, Coords dimensions, float cell_size) {
	// Free existing data
	if(grid->data) 
		free(grid->data);

	if(grid->rotation)
		free(grid->rotation);

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
	new_grid.rotation = calloc(new_grid.cell_count, sizeof(uint8_t));

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

		uint8_t model_id = 0;
		float angle = 0;

		// Set model
		switch(grid->data[cell_id]) {
			case 'x': model_id = 0;	break;
			case 'c': model_id = 1;	break;
		}

		// Set rotation
		switch(grid->rotation[cell_id]) {
			case 0: 	angle = 0;		break;
			case 1:		angle = 90;		break;
			case 2:		angle = 180;	break;
			case 3: 	angle = 270;	break;
		}

		DrawModelShadedEx(map->asset_table[model_id].model, position, CAMERA_UP, angle);
	}	

	if(map->edit_mode == MODE_INSERT) {
		DrawCubeWiresV(CoordsToVec3(hover_coords, grid), cell_size_v, BLUE);
	}

	DrawCubeWiresV(
		Vector3Scale((Vector3){grid->cols - 1, grid->rows - 1, grid->tabs - 1}, grid->cell_size * 0.5f),
		Vector3Scale((Vector3){grid->cols, grid->rows, grid->tabs}, grid->cell_size),
		LIGHTGRAY
	);
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
	if(map->action_count + 1 >= map->action_cap) {
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

	Action undo_action = (Action) {
		.cells = malloc(sizeof(uint32_t) * action->cell_count), 
		.data = malloc(sizeof(unsigned char) * action->cell_count),
		.rotation = calloc(action->cell_count, sizeof(uint8_t)),
		.cell_count = action->cell_count,
		.id = action->id
	};

	for(uint32_t i = 0; i < action->cell_count; i++) {
		uint32_t cell_id = action->cells[i];	

		undo_action.cells[i] = cell_id;
		undo_action.data[i] = map->grid.data[cell_id]; 
		undo_action.rotation[i] = map->grid.rotation[cell_id];

		map->grid.data[cell_id] = action->data[i];
		map->grid.rotation[cell_id] = action->rotation[i];
	}

	map->actions_redo[map->action_count] = *action; 
	map->actions_undo[map->action_count] = undo_action; 

	map->action_count++;
	map->curr_action = map->action_count;
}

void ActionUndo(Map *map) {
	// Prevent undo past first action
	if(map->curr_action < 1) return; 

	map->curr_action--;

	Action *action_undo = &map->actions_undo[map->curr_action];

	// Set data
	for(uint32_t i = 0; i < action_undo->cell_count; i++) {
		uint32_t cell_id = action_undo->cells[i];

		map->grid.data[cell_id] = action_undo->data[i];
		map->grid.rotation[cell_id] = action_undo->rotation[i];
	}
}

void ActionRedo(Map *map) {
	// Prevent redo past last action
	if(map->curr_action > map->action_count - 1) return;

	Action *action_redo = &map->actions_redo[map->curr_action];

	// Set data
	for(uint32_t i = 0; i < action_redo->cell_count; i++) {
		uint32_t cell_id = action_redo->cells[i];

		map->grid.data[cell_id] = action_redo->data[i];
		map->grid.rotation[cell_id] = action_redo->rotation[i];
	}

	map->curr_action++;
}

void ActionFreeData(Action *action) {
	if(action->data)
		free(action->data);

	if(action->rotation)
		free(action->rotation);

	if(action->cells)
		free(action->cells);

	*action = (Action) { 0 };
}

void MapExportLayout(Map *map, char *path) {
	FILE *pF = fopen(path, "w");	

	if(!pF) {
		printf("ERROR: could not write to path: %s\n", path);
		return;
	}

	fprintf(pF, "%d\n", map->grid.cols);	
	fprintf(pF, "%d\n", map->grid.rows);	
	fprintf(pF, "%d\n", map->grid.tabs);	

	for(uint32_t i = 0; i < map->grid.cell_count; i++) 
		fputc(map->grid.data[i], pF);

	fclose(pF);	
}

// Import voxel layout
void MapImportLayout(Map *map, char *path) {
	// Open file at path
	FILE *pF = fopen(path, "r");	

	// Return and log error if file not found
	if(!pF) {
		printf("ERROR: could not read from path: %s\n", path);
		return;
	}

	// TODO:
	// Read file contents,
	// Set grid data

	// Close file
	fclose(pF);
}

void MapExportModel(Map *map, char *path) {
}

