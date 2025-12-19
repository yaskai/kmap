#include <stdint.h>
#include "raylib.h"

#ifndef GUI_H_
#define GUI_H_

typedef struct {
	Rectangle panels[4];

	uint8_t flags;

} Gui;

void GuiInit(Gui *gui);
void GuiUpdate(Gui *gui);

#endif
