#ifndef LIGHTS_H_
#define LIGHTS_H_

#include <stdint.h>
#include "raylib.h"

#define MAX_LIGHTS 16 

#define LIGHT_SELECTED	0x01

typedef enum : uint8_t {
	LIGHT_DEFAULT 	= 0,

} LIGHT_TYPE;

typedef struct {
	Color color;

	Vector3 position;

	float range;

	int enabled;
	int type;
	int id;

	uint8_t flags;

} Light;

typedef struct {
	Light lights[MAX_LIGHTS];
	Shader shader;

	Vector3 ambient_color;

	int light_count;

	int selected_id;

} LightHandler;

void MakeLight(int type, float range, Vector3 position, Color color, LightHandler *handler);
void DeleteLight(LightHandler *handler, uint8_t id);

void InitLights(LightHandler *handler);
void UpdateLights(LightHandler *handler);
void LoadLights(LightHandler *handler, char *file_path);

void DrawModelShaded(Model model, Vector3 position);
void DrawModelShadedEx(Model model, Vector3 position, Vector3 forward, float angle);

void DrawLightGizmos(LightHandler *handler, uint8_t id);

Vector3 ColorQuantized(Color color);

#endif
