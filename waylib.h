#ifndef WAYLIB_H
#define WAYLIB_H
#include <wayland-client.h>

typedef unsigned char waylib_bool;
#define WAYLIB_TRUE (waylib_bool)1
#define WAYLIB_FALSE (waylib_bool)0
#define WAYLIB_BOOL(b) (b) ? WAYLIB_TRUE : WAYLIB_FALSE;

typedef struct waylib_display {
	struct wl_display* dpy;
	struct wl_registry* registry;
	struct wl_compositor *compositor;
	struct wl_subcompositor *subcompositor;
	struct xdg_wm_base *xdg_wm_base;
	struct wl_seat *seat;
	struct wl_shm *shm;
	struct wl_cursor_theme *cursor_theme;
	struct zxdg_decoration_manager_v1* decoration_manager;
	struct xkb_context *xkb_context;
	struct xkb_keymap *keymap;
	struct xkb_state *xkb_state;
	struct zxdg_toplevel_decoration_v1* decoration;
	struct xdg_toplevel* xdg_toplevel;

	void* libdecor; /* libdecor fallback */
	void* decor_ctx;
	waylib_bool manual_decoration;
} waylib_display;

typedef struct waylib_surface {
	struct wl_surface* surface;
} waylib_surface;

typedef struct waylib_window {
	waylib_surface surface;
	struct xdg_surface* xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
} waylib_window;

waylib_bool waylib_display_open(waylib_display* display);
waylib_bool waylib_display_close(waylib_display* display);

waylib_bool waylib_surface_init(waylib_display* display, waylib_surface* surface);
waylib_bool waylib_surface_free(waylib_surface* surface);

waylib_bool waylib_window_init(waylib_display* display, int width, int height, waylib_window* window);
waylib_bool waylib_window_free(waylib_window* window);

#endif
