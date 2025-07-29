#ifndef WAYLIB_C
#define WAYLIB_C

#include "waylib.h"

#include <wayland-util.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1.h"
#include "viewporter-client-protocol.h"

#include "xdg-shell-client-protocol.c"
#include "xdg-decoration-unstable-v1.c"
#include "viewporter-client-protocol.c"

#include <stdint.h>
#include <string.h>

#include <dlfcn.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

struct libdecor;
enum libdecor_error {
	LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
	LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION,
};

struct libdecor_interface {
	/**
	 * An error event
	 */
	void (* error)(struct libdecor *context,
		       enum libdecor_error error,
		       const char *message);

	/* Reserved */
	void (* reserved0)(void);
	void (* reserved1)(void);
	void (* reserved2)(void);
	void (* reserved3)(void);
	void (* reserved4)(void);
	void (* reserved5)(void);
	void (* reserved6)(void);
	void (* reserved7)(void);
	void (* reserved8)(void);
	void (* reserved9)(void);
};

struct libdecor_frame;
struct libdecor_configuration;

/**
 * Interface for integrating a Wayland surface with libdecor.
 */
struct libdecor_frame_interface {
	/**
	 * A new configuration was received. An application should respond to
	 * this by creating a suitable libdecor_state, and apply it using
	 * libdecor_frame_commit.
	 */
	void (* configure)(struct libdecor_frame *frame,
			   struct libdecor_configuration *configuration,
			   void *user_data);

	/**
	 * The window was requested to be closed by the compositor.
	 */
	void (* close)(struct libdecor_frame *frame,
		       void *user_data);

	/**
	 * The window decoration asked to have the main surface to be
	 * committed. This is required when the decoration is implemented using
	 * synchronous subsurfaces.
	 */
	void (* commit)(struct libdecor_frame *frame,
			void *user_data);

	/**
	 * Any mapped popup that has a grab on the given seat should be
	 * dismissed.
	 */
	void (* dismiss_popup)(struct libdecor_frame *frame,
			       const char *seat_name,
			       void *user_data);

	/* Reserved */
	void (* reserved0)(void);
	void (* reserved1)(void);
	void (* reserved2)(void);
	void (* reserved3)(void);
	void (* reserved4)(void);
	void (* reserved5)(void);
	void (* reserved6)(void);
	void (* reserved7)(void);
	void (* reserved8)(void);
	void (* reserved9)(void);
};

typedef struct libdecor* (*libdecor_new_type)(struct wl_display *display, struct libdecor_interface *iface);
typedef void (*libdecor_unref_type)(struct libdecor*);
typedef struct libdecor_frame* (*libdecor_decorate_type)(struct libdecor *context, struct wl_surface *surface, struct libdecor_frame_interface *iface, void *user_data);
typedef void (*libdecor_frame_set_title_type)(struct libdecor_frame* frame, const char* string);
typedef libdecor_frame_set_title_type libdecor_frame_set_app_id_type;
typedef int (*libdecor_dispatch_type)(struct libdecor *context, int time);
typedef int (*libdecor_frame_map_type)(struct libdecor_frame* frame);
typedef int (*libdecor_frame_close_type)(struct libdecor_frame* frame);

libdecor_new_type libdecor_new = NULL;
libdecor_unref_type libdecor_unref = NULL;
libdecor_decorate_type libdecor_decorate = NULL;
libdecor_frame_set_title_type libdecor_frame_set_title = NULL;
libdecor_frame_set_app_id_type libdecor_frame_set_app_id = NULL;
libdecor_dispatch_type libdecor_dispatch = NULL;
libdecor_frame_close_type libdecor_frame_close = NULL;
libdecor_frame_map_type libdecor_frame_map = NULL;

#define WAYLIB_UNUSED(x) (void)(x)

void registry_add_object(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
void registry_remove_object(void *data, struct wl_registry *registry, uint32_t name);
struct wl_registry_listener registry_listener = {&registry_add_object, &registry_remove_object};

void xdg_wm_base_ping_handler(void* data, struct xdg_wm_base* wm_base, uint32_t serial) {
    xdg_wm_base_pong(wm_base, serial);
}

void xdg_surface_configure_handler(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);
}

void xdg_toplevel_configure_handler(void* data, struct xdg_toplevel* toplevel, int32_t width, int32_t height, struct wl_array* states) {
    waylib_display* display = (waylib_display*)data;
}

void xdg_toplevel_close_handler(void* data, struct xdg_toplevel *toplevel) {
	waylib_display* display = (waylib_display*)data;
}

void xdg_decoration_configure_handler(void* data, struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1, uint32_t mode) {
	zxdg_toplevel_decoration_v1_set_mode(zxdg_toplevel_decoration_v1, mode);
}

void surface_frame_done(void* data, struct wl_callback *cb, uint32_t time) {

}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
	.ping = xdg_wm_base_ping_handler,
};

#define WAYLIB_PROC_DEF(proc, name) if (name == NULL && proc != NULL) { \
	void* ptr = dlsym(proc, #name); \
	if (ptr != NULL) memcpy(&name, &ptr, sizeof(name##_type)); \
}

void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;

    int i;
    for (i = 0; i < 6; ++i) {
		buf[i] = (char)('A'+(r&15)+(r&16)*2);
		r >>= 5;
	}
}

int anonymous_shm_open(void) {
	char name[] = "/wayland-XXXXXX";
	int retries = 100;

	do {
		randname(name + strlen(name) - 6);

		--retries;
		/* shm_open guarantees that O_CLOEXEC is set */
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			shm_unlink(name);
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);

	return -1;
}

waylib_bool waylib_shm_init(waylib_display* display, unsigned long size, waylib_shm* mem) {
	mem->display = display;
	mem->size = size;
	mem->fd = anonymous_shm_open();
	if (mem->fd < 0) {
		return WAYLIB_FALSE;
	}

	if (ftruncate(mem->fd, size) < 0) {
		close(mem->fd);
		return -1;
	}

	if (mem->fd < 0) {
		return WAYLIB_FALSE;
	}

	mem->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem->fd, 0);
	if (mem->data == MAP_FAILED) {
		return WAYLIB_FALSE;
	}
	return WAYLIB_TRUE;
}

waylib_bool waylib_shm_free(waylib_shm* mem) {
	munmap(mem->data, mem->size);
	close(mem->fd);
	return WAYLIB_TRUE;
}

waylib_bool waylib_buffer_init_with_shm(waylib_shm* mem, int width, int height, int stride, unsigned int format, waylib_buffer* buffer) {
	struct wl_shm_pool* pool = wl_shm_create_pool(mem->display->shm, mem->fd, mem->size);
	buffer->buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, format);
	wl_shm_pool_destroy(pool);
	return WAYLIB_TRUE;
}

waylib_bool waylib_buffer_free(waylib_buffer* buffer) {
	wl_buffer_destroy(buffer->buffer);
	return WAYLIB_TRUE;
}



waylib_bool waylib_display_init(waylib_display* display) {
    memset(display, 0, sizeof(waylib_display));
    display->dpy = wl_display_connect(NULL);
	if (display->dpy == NULL) {
		return WAYLIB_FALSE;
	}

    display->registry = wl_display_get_registry(display->dpy);

    wl_registry_add_listener(display->registry, &registry_listener, display);

	display->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	if (display->decoration_manager == NULL) {
        display->libdecor = dlopen("libdecor-0.so.0", RTLD_LAZY | RTLD_LOCAL);
        if (display->libdecor) {
			WAYLIB_PROC_DEF(display->libdecor, libdecor_new);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_unref);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_new);
            WAYLIB_PROC_DEF(display->libdecor, libdecor_decorate);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_frame_set_title);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_frame_set_app_id);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_dispatch);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_frame_map);
			WAYLIB_PROC_DEF(display->libdecor, libdecor_frame_close);
		}

        if (libdecor_new &&  libdecor_unref && libdecor_decorate) {
            static struct libdecor_interface interface = {
				.error = NULL,
			};

            if (libdecor_new)
    			display->decor_ctx = libdecor_new(display->dpy, &interface);
	    } else {
            display->manual_decoration = WAYLIB_TRUE;
        }
    }

	waylib_display_roundtrip(display, NULL);
	waylib_display_dispatch(display, NULL);

	return WAYLIB_TRUE;
}

waylib_bool waylib_display_roundtrip(waylib_display* display, int* event_count) {
	int cnt = wl_display_roundtrip(display->dpy);
	if (event_count) *event_count = cnt;
	return (cnt >= 0);
}

waylib_bool waylib_display_dispatch(waylib_display* display, int* event_count) {
	int cnt1 = 0;
	if (libdecor_dispatch)
		cnt1 = libdecor_dispatch(display->decor_ctx, 0);
	int cnt2 = wl_display_dispatch(display->dpy);

	if (event_count) {
		if (cnt1 >= 0 && cnt2 >= 0)
			*event_count = cnt1 + cnt2;
		else
			*event_count = -1;
	}
	return (cnt1 >= 0) && (cnt2 > 0);
}

waylib_bool waylib_display_flush(waylib_display* display, int* bytes) {
	int cnt = wl_display_flush(display->dpy);
	if (bytes) *bytes = cnt;
	return (cnt >= 0);
}

waylib_bool waylib_display_close(waylib_display* display) {
    if (display->libdecor && display->decor_ctx && libdecor_unref) {
        libdecor_unref(display->decor_ctx);
    }

	if (display->decoration_manager != NULL) {
		zxdg_decoration_manager_v1_destroy(display->decoration_manager);
	}

    if (display->cursor_theme) {
        wl_cursor_theme_destroy(display->cursor_theme);
    }

    if (display->registry) {
        wl_registry_destroy(display->registry);
    }

    wl_display_disconnect(display->dpy);
	return WAYLIB_TRUE;
}

void wl_surface_frame_done(void* data, struct wl_callback *cb, uint32_t time) {

}

const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure_handler,
};

const struct wl_callback_listener wl_surface_frame_listener = {
	.done = wl_surface_frame_done,
};

const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure_handler,
	.close = xdg_toplevel_close_handler,
};

waylib_bool waylib_subsurface_init(waylib_display* display,  waylib_surface* surface, waylib_surface* parent, waylib_subsurface* subsurface) {
	subsurface->subsurface = wl_subcompositor_get_subsurface(display->subcompositor, surface->surface, parent->surface);
	return WAYLIB_TRUE;
}

waylib_bool waylib_subsurface_set_position(waylib_subsurface* subsurface, int x, int y) {
	wl_subsurface_set_position(subsurface->subsurface, x, y);
	return WAYLIB_TRUE;
}

waylib_bool waylib_subsurface_free(waylib_subsurface* subsurface) {
	wl_subsurface_destroy(subsurface->subsurface);
	return WAYLIB_TRUE;
}

waylib_bool waylib_viewport_init(waylib_display* display, waylib_surface* surface, waylib_viewport* viewport) {
	viewport->viewport = wp_viewporter_get_viewport(display->viewporter, surface->surface);
	return WAYLIB_TRUE;
}

waylib_bool waylib_viewport_set_destination(waylib_viewport* viewport, int width, int height) {
	wp_viewport_set_destination(viewport->viewport, width, height);
	return WAYLIB_TRUE;
}

waylib_bool waylib_viewport_free(waylib_viewport* viewport) {
	wp_viewport_destroy(viewport->viewport);
	return WAYLIB_TRUE;
}

waylib_bool waylib_region_init(waylib_display* display, waylib_region* region) {
	region->region = wl_compositor_create_region(display->compositor);
	return WAYLIB_TRUE;
}

waylib_bool waylib_region_add(waylib_region* region, int x, int y, int width, int height) {
	wl_region_add(region->region, x, y, width, height);
	return WAYLIB_TRUE;
}

waylib_bool waylib_region_free(waylib_region* region) {
	wl_region_destroy(region->region);
	return WAYLIB_TRUE;
}


void create_fallback_edge(waylib_window* window, waylib_fallbackEdge* edge, waylib_surface* parent, waylib_buffer* buffer, int x, int y, int width, int height) {
	waylib_surface_init(window->display, &edge->surface);
	waylib_subsurface_init(window->display, &edge->surface, parent, &edge->subsurface);
	wl_surface_set_user_data(edge->surface.surface, window);
//    wl_proxy_set_tag((struct wl_proxy*) edge->surface, &display->tag);
	waylib_subsurface_set_position(&edge->subsurface, x, y);
	waylib_viewport_init(window->display, &edge->surface, &edge->viewport);
	waylib_viewport_set_destination(&edge->viewport, width, height);
	waylib_surface_attach(&edge->surface, buffer, 0, 0);

	waylib_region region;
	waylib_region_init(window->display, &region);
	waylib_region_add(&region, 0, 0, width, height);
    waylib_surface_set_opaque_region(&edge->surface, &region);
    waylib_surface_commit(&edge->surface);
	waylib_region_free(&region);
}

#define WAYLIB_BORDER_SIZE    4
#define WAYLIB_CAPTION_HEIGHT 24

void create_fallback_decorations(waylib_window* window, int width, int height) {
    unsigned char data[] = { 224, 224, 224, 255 };

    if (!window->fallback.buffer.buffer) {
		waylib_shm_init(window->display, 4, &window->fallback.shm);
		waylib_buffer_init_with_shm(&window->fallback.shm, 1, 1, 1 * 4, WL_SHM_FORMAT_ARGB8888, &window->fallback.buffer);
	}
    if (!window->fallback.buffer.buffer)
        return;

    create_fallback_edge(window, &window->fallback.top, window->surface,
                       &window->fallback.buffer,
                       0, -WAYLIB_CAPTION_HEIGHT,
                       width, WAYLIB_CAPTION_HEIGHT);
    create_fallback_edge(window, &window->fallback.left, window->surface,
                       &window->fallback.buffer,
                       -WAYLIB_BORDER_SIZE, -WAYLIB_CAPTION_HEIGHT,
                       WAYLIB_BORDER_SIZE, height + WAYLIB_CAPTION_HEIGHT);
    create_fallback_edge(window, &window->fallback.right, window->surface,
                       &window->fallback.buffer,
                       width, -WAYLIB_CAPTION_HEIGHT,
                       WAYLIB_BORDER_SIZE, height + WAYLIB_CAPTION_HEIGHT);
    create_fallback_edge(window, &window->fallback.bottom, window->surface,
                       &window->fallback.buffer,
                       -WAYLIB_BORDER_SIZE, height,
                       width + WAYLIB_BORDER_SIZE * 2, WAYLIB_BORDER_SIZE);

//    window->fallback.decorations = WAYLIB_TRUE;
}

waylib_bool waylib_window_init(waylib_display* display, waylib_surface* surface, int width, int height, waylib_window* window) {
	memset(window, 0, sizeof(waylib_window));
	window->surface = surface;
	window->display = display;

	window->decoration = NULL;
	window->decor_frame = NULL;
	if (libdecor_new &&  libdecor_unref) {
        static struct libdecor_frame_interface frameInterface = {0}; /*= {
            wl_handle_configure,
            wl_handle_close,
            wl_handle_commit,
            wl_handle_dismiss_popup,
        };*/

//	    window->decor_frame = libdecor_decorate(display->decor_ctx, window->surface.surface, &frameInterface, window);
//		libdecor_frame_map(window->decor_frame);
	}

	if (window->decor_frame == NULL) {
		window->xdg_surface = xdg_wm_base_get_xdg_surface(display->xdg_wm_base, window->surface->surface);
		window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
		xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
		xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

		xdg_surface_set_window_geometry(window->xdg_surface, 0, 0, width, height);

		if (display->decoration_manager) {
			window->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(display->decoration_manager, window->xdg_toplevel);

			static const struct zxdg_toplevel_decoration_v1_listener xdg_decoration_listener = {
					.configure = xdg_decoration_configure_handler
			};

			zxdg_toplevel_decoration_v1_add_listener(window->decoration, &xdg_decoration_listener, NULL);
		} else {
			create_fallback_decorations(window, width, height);
		}
	}

	struct wl_callback* callback = wl_surface_frame(window->surface->surface);
	wl_callback_add_listener(callback, &wl_surface_frame_listener, window);

	wl_surface_commit(window->surface->surface);
	waylib_display_flush(display, NULL);
	return WAYLIB_TRUE;
}

waylib_bool waylib_window_set_app_id(waylib_window* window, const char* name) {
	if (window->decor_frame) {
		libdecor_frame_set_app_id(window->decor_frame, name);
	} else {
		xdg_toplevel_set_app_id(window->xdg_toplevel, name);
	}

	return WAYLIB_TRUE;
}

waylib_bool waylib_window_set_title(waylib_window* window, const char* name) {
	if (window->decor_frame) {
		libdecor_frame_set_title(window->decor_frame, name);
	} else {
		xdg_toplevel_set_title(window->xdg_toplevel, name);
	}

	return WAYLIB_TRUE;
}


waylib_bool waylib_window_free(waylib_window* window) {
	if (window->decor_frame) {
		libdecor_frame_close(window->decor_frame);
	}

	return WAYLIB_TRUE;
}

waylib_bool waylib_surface_init(waylib_display* display, waylib_surface* surface) {
	surface->surface = wl_compositor_create_surface(display->compositor);
    return (surface->surface == NULL);
}

waylib_bool waylib_surface_attach(waylib_surface* surface, waylib_buffer* buffer, int x, int y) {
	wl_surface_attach(surface->surface, buffer->buffer, x, y);
    return WAYLIB_TRUE;
}

waylib_bool waylib_surface_set_opaque_region(waylib_surface* surface, waylib_region* region) {
	wl_surface_set_opaque_region(surface->surface, region->region);
    return WAYLIB_TRUE;
}

waylib_bool waylib_surface_commit(waylib_surface* surface) {
	wl_surface_commit(surface->surface);
    return WAYLIB_TRUE;
}

waylib_bool waylib_surface_free(waylib_surface* surface) {
    wl_surface_destroy(surface->surface);
    return WAYLIB_TRUE;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {

}

void pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {

}

void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {

}

void pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {

}


void pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {

}

void keyboard_keymap (void* data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {

}

void keyboard_enter (void* data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {

}

void keyboard_leave (void* data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {

}

void keyboard_key (void* data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {

}

void keyboard_modifiers (void* data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {

}

void seat_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
    waylib_display* display = (waylib_display*) data;

    static struct wl_pointer_listener pointer_listener;
	memset(&pointer_listener, 0, sizeof (pointer_listener));
	pointer_listener.enter = &pointer_enter;
	pointer_listener.leave = &pointer_leave;
	pointer_listener.motion = &pointer_motion;
	pointer_listener.button = &pointer_button;
	pointer_listener.axis = &pointer_axis;

	static struct wl_keyboard_listener keyboard_listener;
	memset(&keyboard_listener, 0, sizeof (keyboard_listener));
	keyboard_listener.keymap = &keyboard_keymap;
	keyboard_listener.enter = &keyboard_enter;
	keyboard_listener.leave = &keyboard_leave;
	keyboard_listener.key = &keyboard_key;
	keyboard_listener.modifiers = &keyboard_modifiers;

    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        struct wl_pointer *pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, data);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
        wl_keyboard_add_listener (keyboard, &keyboard_listener, NULL);
    }
}

void shm_format_handler(void* data, struct wl_shm *shm, uint32_t format) {
}

void do_nothing(void) { }

void registry_add_object(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    static struct wl_seat_listener seat_listener = {&seat_capabilities, (void (*)(void *, struct wl_seat *, const char *))&do_nothing};
    static const struct wl_shm_listener shm_listener = { .format = shm_format_handler };

	waylib_display* display = (waylib_display*)data;
	if (!strcmp(interface, wl_compositor_interface.name)) {
        display->compositor = wl_registry_bind (registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
		display->subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
    } else if (!strcmp(interface, wl_seat_interface.name)) {
		display->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener (display->seat, &seat_listener, data);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
		display->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		wl_shm_add_listener(display->shm, &shm_listener, NULL)	;
		display->cursor_theme = wl_cursor_theme_load(NULL, 32, display->shm);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		display->xdg_wm_base = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, MIN(version, 1));
		xdg_wm_base_add_listener(display->xdg_wm_base, &xdg_wm_base_listener, display);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		display->decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, "wp_viewporter") == 0) {
        display->viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
    }
}

void registry_remove_object(void *data, struct wl_registry *registry, uint32_t name) {
	/* do nothing... */
}

#endif /* WAYLIB_C */
