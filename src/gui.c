#include "raylib.h"
#include "gui.h"

#define RAYGUI_IMPLEMENTATION
#include "include/raygui.h"
#undef RAYGUI_IMPLEMENTATION

void GuiInit(Gui *gui) {
}

void GuiUpdate(Gui *gui) {
	// Draw mouse pointer
	Vector2 mouse_pos = GetMousePosition();
	Color color = RAYWHITE;
	int icon = ICON_CURSOR_CLASSIC;
	GuiDrawIcon(icon, mouse_pos.x, mouse_pos.y, 2, color);
}
