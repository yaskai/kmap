#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "lights.h"

Color LIGHT_COLOR_DEFAULT;

int enabled_loc;
int positions_loc;
int colors_loc;
int ranges_loc;
int count_loc;
int time_loc;
int ambient_loc;

float ent_light_timer = 0.0f;

Shader light_shader;
LightHandler *lh;

void MakeLight(int type, float range, Vector3 position, Color color, LightHandler *handler) {
	Light *light = &handler->lights[handler->light_count++];
	light->flags = 0;
	light->type = type;
	light->range = range;
	light->position = position;
	light->color = color;
	light->enabled = 1;
}

void DeleteLight(LightHandler *handler, uint8_t id) {
	if(id >= handler->light_count) return;

	for(uint8_t i = id; i < handler->light_count - 1; i++) 
		handler->lights[i] = handler->lights[i + 1];

	handler->light_count--;

	for(uint8_t i = 0; i < handler->light_count; i++)
		handler->lights[i].flags &= ~LIGHT_SELECTED;

	handler->selected_id = -1;
}

void InitLights(LightHandler *handler) {
	lh = handler;

	handler->light_count = 0;
	//LIGHT_COLOR_DEFAULT = ColorBrightness(BEIGE, -0.25f);
	LIGHT_COLOR_DEFAULT = ColorBrightness(BEIGE, 0.25f);

	handler->shader = LoadShader(
		TextFormat("resources/shaders/light_v.glsl"),
		TextFormat("resources/shaders/light_f.glsl")
		//TextFormat("shaders/debug_f.glsl")
	);

	light_shader = handler->shader;
	
	enabled_loc 	= GetShaderLocation(handler->shader, "light_enabled");
	positions_loc 	= GetShaderLocation(handler->shader, "light_positions");
	colors_loc 		= GetShaderLocation(handler->shader, "light_colors");
	ranges_loc 		= GetShaderLocation(handler->shader, "light_ranges");
	count_loc 		= GetShaderLocation(handler->shader, "light_count");
	time_loc 		= GetShaderLocation(handler->shader, "time");
	ambient_loc 	= GetShaderLocation(handler->shader, "ambient");

	// Static lights
	/*
	MakeLight(LIGHT_DEFAULT, 35.0f, (Vector3){   0.0f,   8.0f,   0.0f }, LIGHT_COLOR_DEFAULT, 			handler);
	MakeLight(LIGHT_DEFAULT, 20.0f, (Vector3){ -40.0f,   3.0f,   0.0f }, LIGHT_COLOR_DEFAULT, 			handler);
	MakeLight(LIGHT_DEFAULT, 20.0f, (Vector3){ -40.0f,   3.0f, -15.0f }, LIGHT_COLOR_DEFAULT,			handler);
	MakeLight(LIGHT_DEFAULT, 20.0f, (Vector3){   0.0f,   3.0f, -44.0f }, LIGHT_COLOR_DEFAULT, 			handler);
	MakeLight(LIGHT_DEFAULT, 10.0f, (Vector3){ -65.0f,   3.0f,  15.0f }, LIGHT_COLOR_DEFAULT, 			handler);
	*/

	//MakeLight(LIGHT_PLAYER, 10.0f, Vector3Zero(), LIGHT_COLOR_DEFAULT, handler);

	int enabled[MAX_LIGHTS];
	Vector3 positions[MAX_LIGHTS];
	Vector3 colors[MAX_LIGHTS];
	float ranges[MAX_LIGHTS];

	Color ambient_color = ColorBrightness(WHITE, -0.75f);
	Vector3 ambient = ColorQuantized(ambient_color);
	handler->ambient_color = ambient;

	for(int i = 0; i < handler->light_count; i++) {
		enabled[i] = 1;	
		positions[i] = handler->lights[i].position;

		colors[i] = (Vector3) {
			handler->lights[i].color.r / 255.0f,
			handler->lights[i].color.g / 255.0f,
			handler->lights[i].color.b / 255.0f
		};
		
		ranges[i] = handler->lights[i].range;
	}

	int count = handler->light_count;
	
	SetShaderValueV(handler->shader, enabled_loc, enabled, SHADER_UNIFORM_INT, count);
	SetShaderValueV(handler->shader, positions_loc, positions, SHADER_UNIFORM_VEC3, count);
	SetShaderValueV(handler->shader, colors_loc, colors, SHADER_UNIFORM_VEC3, count);
	SetShaderValueV(handler->shader, ranges_loc, ranges, SHADER_UNIFORM_FLOAT, count);
	SetShaderValue(handler->shader, count_loc, &count, SHADER_UNIFORM_INT);
	SetShaderValue(handler->shader, ambient_loc, &ambient, SHADER_UNIFORM_VEC3);

	Vector4 diffuse = (Vector4){ 0.55f, 0.15f, 0.15f, 1.0f };
	SetShaderValue(handler->shader, GetShaderLocation(handler->shader, "col_diffuse"), &diffuse, SHADER_UNIFORM_VEC4);
	
	handler->selected_id = -1;

	printf("light count: %d\n", handler->light_count);
	for(int i = 0; i < handler->light_count; i++) {
		printf("light[%d] enabled: %d\n", i, handler->lights[i].enabled);
	}
}

void UpdateLights(LightHandler *handler) {
	float time = GetTime();
	SetShaderValue(handler->shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

	int enabled[handler->light_count];
	Vector3 positions[handler->light_count];
	float ranges[handler->light_count];
	Vector3 colors[handler->light_count];
	
	SetShaderValue(handler->shader, count_loc, &handler->light_count, SHADER_UNIFORM_INT);

	for(int i = 0; i < handler->light_count; i++) {
		// Set on/off
		enabled[i] = handler->lights[i].enabled;
		SetShaderValueV(handler->shader, enabled_loc, enabled, SHADER_UNIFORM_INT, handler->light_count);

		// Set positions
		positions[i] = handler->lights[i].position;
		SetShaderValueV(handler->shader, positions_loc, positions, SHADER_UNIFORM_VEC3, handler->light_count);

		// Set ranges
		ranges[i] = handler->lights[i].range;		
		SetShaderValueV(handler->shader, ranges_loc, ranges, SHADER_UNIFORM_FLOAT, handler->light_count);

		colors[i] = ColorQuantized(handler->lights[i].color);
		SetShaderValueV(handler->shader, colors_loc, colors, SHADER_UNIFORM_VEC3, handler->light_count);
	}
}

void DrawModelShaded(Model model, Vector3 position) {
	BeginShaderMode(light_shader);
	Matrix mat = model.transform;
	mat = MatrixTranslate(position.x, position.y, position.z);	
	int mat_model_loc = GetShaderLocation(light_shader, "mat_model");
	SetShaderValueMatrix(light_shader, mat_model_loc, mat);

	DrawModel(model, position, 1.0f, WHITE);
	EndShaderMode();
}

void DrawModelShadedEx(Model model, Vector3 position, Vector3 forward, float angle) {
	BeginShaderMode(light_shader);	

	Matrix mat = model.transform;
	mat = MatrixMultiply(mat, MatrixTranslate(position.x, position.y, position.z));
	//mat = MatrixMultiply(mat, MatrixRotateY(angle * DEG2RAD));

	int mat_model_loc = GetShaderLocation(light_shader, "mat_model");
	SetShaderValueMatrix(light_shader, mat_model_loc, mat);

	DrawModelEx(model, position, (Vector3){0, 1, 0}, angle, Vector3One(), WHITE);
	EndShaderMode();
}

void DrawLightGizmos(LightHandler *handler, uint8_t id) {
	Light *light = &handler->lights[id];

	DrawSphere(light->position, 0.5f, ColorAlpha(light->color, 0.95f));

	if(light->flags & LIGHT_SELECTED) {
		DrawSphere(light->position, 0.5f, ColorAlpha(ORANGE, 0.95f));

		DrawLine3D(light->position, Vector3Add(light->position, Vector3Scale((Vector3){1, 0, 0}, 2.0f)), RED);
		DrawLine3D(light->position, Vector3Add(light->position, Vector3Scale((Vector3){0, 1, 0}, 2.0f)), GREEN);
		DrawLine3D(light->position, Vector3Add(light->position, Vector3Scale((Vector3){0, 0, 1}, 2.0f)), BLUE);
		
		DrawSphereWires(light->position, light->range * 0.25f, 8, 64, ColorAlpha(light->color, 0.5f));
	}
}

Vector3 ColorQuantized(Color color) {
	return (Vector3) {
		color.r / 255.0f,
		color.g / 255.0f,
		color.b / 255.0f
	};
}
 
