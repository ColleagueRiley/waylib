#define WAYLIB_API inline
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
	waylib_surface surface;

	waylib_surface_init(&display, &surface);
	waylib_window_init(&display, &surface, 500, 500, &window);

	waylib_window_set_app_id(&window, "app");
	waylib_window_set_title(&window, "window");

	while (1) {
		waylib_display_dispatch(&display, NULL);
	}

	waylib_surface_free(&surface);
	waylib_window_free(&window);
	waylib_display_close(&display);
}
