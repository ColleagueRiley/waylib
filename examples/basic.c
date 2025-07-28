#include "waylib.h"
#include "waylib.c"

#include <stdio.h>

int main(void) {
	waylib_display display;
	if (waylib_display_open(&display) == WAYLIB_FALSE) {
		printf("failed to create a wayland display\n");
		return 0;
	}

	waylib_window window;
	waylib_window_init(&display, 500, 500, &window);

	waylib_window_free(&window);
	waylib_display_close(&display);
}
