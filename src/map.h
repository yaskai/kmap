#include <stdint.h>
#include "raylib.h"

#ifndef MAP_H_
#define MAP_H_

typedef struct {
	int16_t c;	// x, column
	int16_t r;	// y, row
	int16_t t;	// z, tab

} Coords;

typedef struct {
	int32_t *draw_list;

	unsigned char *data;

	float cell_size;

	int32_t cell_count;
	int32_t draw_count;

	int16_t cols;	// width
	int16_t rows;	// height
	int16_t tabs;	// depth
	
} Grid;

typedef struct {
	Model model;
	Material material;

} Asset;

enum ACTION_TYPES : uint8_t {
	ACTION_ADD,
	ACTION_MODIFY
};

typedef struct {
	uint32_t *cells;

	uint32_t cell_count;
	uint32_t id;

	uint8_t type;

} Action;

enum EDITOR_MODES : uint8_t {
	MODE_NORMAL,
	MODE_INSERT
};

#define EXIT_REQUEST	0x01

typedef struct {
	Grid grid;

	Camera3D camera;

	Action *actions_redo;
	Action *actions_undo;

	Asset *asset_table;

	uint32_t curr_action;

	uint8_t flags;
	uint8_t edit_mode;

} Map;

void MapInit(Map *map);
void MapClose(Map *map);
void MapUpdate(Map *map, float dt);
void MapDraw(Map *map);

void GenerateAssetTable(Map *map, char *path);

void GridInit(Grid *grid, Coords dimensions, float cell_size);

int16_t CellCoordsToId(Coords coords, Grid *grid);
Coords CellIdToCoords(int16_t id, Grid *grid);

Coords Vec3ToCoords(Vector3 v, Grid *grid);
Vector3 CoordsToVec3(Coords coords, Grid *grid);

void UpdateDrawList(Map *map, Grid *grid);

#define DCELLS_DRAW_BOXES	0x01
#define DCELLS_OCCLUSION	0x02
#define DCELLS_ONLY_FLOOR	0x04
void DrawCells(Map *map, Grid *grid, uint8_t _pool_);

#define CAMERA_UP				 (Vector3) { 0, 1, 0 }
#define CAMERA_SPEED							50.00f
#define CAMERA_SENSITIVITY						0.275f
void CameraControls(Map *map, float dt);

void ActionApply(Action *action, Map *map);
void ActionUndo(Map *map);
void ActionRedo(Map *map);

#endif

