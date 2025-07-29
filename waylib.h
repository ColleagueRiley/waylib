#ifndef WAYLIB_H
#define WAYLIB_H

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
typedef struct waylib_subsurface waylib_subsurface;
typedef struct waylib_viewport waylib_viewport;
typedef struct waylib_window waylib_window;
typedef struct waylib_buffer waylib_buffer;
typedef struct waylib_region waylib_region;

/* shared memory stucture */
typedef struct waylib_shm {
	waylib_display* display;
	int fd;
	void* data;
	unsigned long size;
} waylib_shm;
/* create a block of shared memory the compositor */
WAYLIB_API waylib_bool waylib_shm_init(waylib_display* display, unsigned long size, waylib_shm* mem);
/* free shared memory */
WAYLIB_API waylib_bool waylib_shm_free(waylib_shm* mem);

/* create a wl_buffer object using a block shared memory (the shared memory must be allocated using waylib_shm_init) */
WAYLIB_API waylib_bool waylib_buffer_init_with_shm(waylib_shm* mem, int width, int height, int stride, unsigned int format, waylib_buffer* buffer);
/* free wl_buffer */
WAYLIB_API waylib_bool waylib_buffer_free(waylib_buffer* buffer);


/* free resources and close the connection to the waylib server and compositor */
WAYLIB_API waylib_bool waylib_display_init(waylib_display* display);
/* Block until all pending request are processed by the server. */
WAYLIB_API waylib_bool waylib_display_roundtrip(waylib_display* display, int* event_count);
/* Process incoming events */
WAYLIB_API waylib_bool waylib_display_dispatch(waylib_display* display, int* event_count);
/* Send all buffered requests on the display to the server */
WAYLIB_API waylib_bool waylib_display_flush(waylib_display* display, int* bytes);
/* free resources and close the connection to the waylib servera and ompositor */
WAYLIB_API waylib_bool waylib_display_close(waylib_display* display);


WAYLIB_API waylib_bool waylib_surface_init(waylib_display* display,  waylib_surface* surface);
/* attach a buffer to your wayland surface */
WAYLIB_API waylib_bool waylib_surface_attach(waylib_surface* surface, waylib_buffer* buffer, int x, int y);
WAYLIB_API waylib_bool waylib_surface_set_opaque_region(waylib_surface* surface, waylib_region* region);
WAYLIB_API waylib_bool waylib_surface_commit(waylib_surface* surface);
WAYLIB_API waylib_bool waylib_surface_free(waylib_surface* surface);

/* turn a surface into a subsurface form the parent surface */
WAYLIB_API waylib_bool waylib_subsurface_init(waylib_display* display,  waylib_surface* surface, waylib_surface* parent, waylib_subsurface* subsurface);
/* set the position of the subsurface on the surface */
WAYLIB_API waylib_bool waylib_subsurface_set_position(waylib_subsurface* subsurface, int x, int y);
/* free the subsurface */
WAYLIB_API waylib_bool waylib_subsurface_free(waylib_subsurface* subsurface);

WAYLIB_API waylib_bool waylib_viewport_init(waylib_display* display, waylib_surface* surface, waylib_viewport* viewport);
WAYLIB_API waylib_bool waylib_viewport_set_destination(waylib_viewport* viewport, int width, int height);
WAYLIB_API waylib_bool waylib_viewport_free(waylib_viewport* viewport);

WAYLIB_API waylib_bool waylib_region_init(waylib_display* display, waylib_region* region);
WAYLIB_API waylib_bool waylib_region_add(waylib_region* region, int x, int y, int width, int height);
WAYLIB_API waylib_bool waylib_region_free(waylib_region* region);

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
	struct wp_viewporter* viewporter;
};

struct waylib_buffer {
	struct wl_buffer* buffer;
};

struct waylib_surface {
	struct wl_surface* surface;
};

struct waylib_subsurface {
	struct wl_subsurface* subsurface;
};

struct waylib_viewport {
    struct wp_viewport* viewport;
};


typedef struct waylib_region {
	struct wl_region* region;
} waylib_region;

/* final decoration fallback (xdg decoration and libdecor both don't exist) */
typedef struct waylib_fallbackEdge{
	waylib_surface surface;
	waylib_subsurface subsurface;
	waylib_viewport viewport;
} waylib_fallbackEdge;

typedef struct waylib_fallback_decoration {
	waylib_shm shm;
	waylib_buffer buffer;
	waylib_fallbackEdge    top, left, right, bottom;
} waylib_fallback_decoration;

struct waylib_window {
	waylib_display* display;
	waylib_surface* surface;
	void* decor_frame;
	struct xdg_surface* xdg_surface;
	struct xdg_toplevel* xdg_toplevel;
	struct zxdg_toplevel_decoration_v1* decoration;
	waylib_fallback_decoration fallback;
};

#endif
