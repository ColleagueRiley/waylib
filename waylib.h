#ifndef WAYLIB_H
#define WAYLIB_H
#include <wayland-client.h>

#if defined(WAYLIB_EXPORT) ||  defined(WAYLIB_IMPORT)
	#if defined(_WIN32)
		#if defined(__TINYC__) && (defined(WAYLIB_EXPORT) ||  defined(WAYLIB_IMPORT))
			#define __declspec(x) __attribute__((x))
		#endif

		#if defined(WAYLIB_EXPORT)
			#define WAYLIB_API __declspec(dllexport)
		#else
			#define WAYLIB_API __declspec(dllimport)
		#endif
	#else
		#if defined(WAYLIB_EXPORT)
			#define WAYLIB_API __attribute__((visibility("default")))
		#endif
	#endif
#endif

#ifndef WAYLIB_API
	#define WAYLIB_API
#endif


typedef unsigned char waylib_bool;
#define WAYLIB_TRUE (waylib_bool)1
#define WAYLIB_FALSE (waylib_bool)0
#define WAYLIB_BOOL(b) (b) ? WAYLIB_TRUE : WAYLIB_FALSE;

typedef struct waylib_display waylib_display;
typedef struct waylib_surface waylib_surface;
typedef struct waylib_window waylib_window;

/* free resources and close the connection to the waylib server and compositor */
WAYLIB_API waylib_bool waylib_display_open(waylib_display* display);
/* Block until all pending request are processed by the server. */
WAYLIB_API waylib_bool waylib_display_roundtrip(waylib_display* display, int* event_count);
/* Process incoming events */
WAYLIB_API waylib_bool waylib_display_dispatch(waylib_display* display, int* event_count);
/* Send all buffered requests on the display to the server */
WAYLIB_API waylib_bool waylib_display_flush(waylib_display* display, int* bytes);
/* free resources and close the connection to the waylib servera and ompositor */
WAYLIB_API waylib_bool waylib_display_close(waylib_display* display);


WAYLIB_API waylib_bool waylib_surface_init(waylib_display* display,  waylib_surface* surface);
WAYLIB_API waylib_bool waylib_surface_free(waylib_surface* surface);

WAYLIB_API waylib_bool waylib_window_init(waylib_display* display, waylib_surface* surface, int width, int height, waylib_window* window);
WAYLIB_API waylib_bool waylib_window_set_app_id(waylib_window* window, const char* name);
WAYLIB_API waylib_bool waylib_window_set_title(waylib_window* window, const char* name);
WAYLIB_API waylib_bool waylib_window_free(waylib_window* window);

struct waylib_display {
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
	void* libdecor; /* libdecor fallback */
	void* decor_ctx;
	waylib_bool manual_decoration;
};

struct waylib_surface {
	struct wl_surface* surface;
};

struct waylib_window {
	waylib_display* display;
	waylib_surface* surface;
	void* decor_frame;
	struct xdg_surface* xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	struct zxdg_toplevel_decoration_v1* decoration;
};

#endif
