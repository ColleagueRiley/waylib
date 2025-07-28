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

#include "xdg-shell-client-protocol.c"
#include "xdg-decoration-unstable-v1.c"

#include <stdint.h>
#include <string.h>

#include <dlfcn.h>

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

libdecor_new_type libdecor_new = NULL;
libdecor_unref_type libdecor_unref = NULL;
libdecor_decorate_type libdecor_decorate = NULL;

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
    waylib_display* display = (waylib_display*)xdg_toplevel_get_user_data(toplevel);
}

void xdg_toplevel_close_handler(void* data, struct xdg_toplevel *toplevel) {
	waylib_display* display = (waylib_display*)xdg_toplevel_get_user_data(toplevel);
}

void xdg_decoration_configure_handler(void* data, struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1, uint32_t mode) {
	zxdg_toplevel_decoration_v1_set_mode(zxdg_toplevel_decoration_v1, mode);
}

void surface_frame_done(void* data, struct wl_callback *cb, uint32_t time) {

}

waylib_bool waylib_display_open(waylib_display* display) {
    memset(display, 0, sizeof(waylib_display));
    display->dpy = wl_display_connect(NULL);
	if (display->dpy == NULL) {
		return WAYLIB_FALSE;
	}

    display->registry = wl_display_get_registry(display->dpy);
    wl_registry_add_listener(display->registry, &registry_listener, display);

    wl_display_roundtrip(display->dpy);
    wl_display_dispatch(display->dpy);

	static const struct xdg_wm_base_listener xdg_wm_base_listener = {
		.ping = xdg_wm_base_ping_handler,
	};

	static const struct xdg_surface_listener xdg_surface_listener = {
		.configure = xdg_surface_configure_handler,
	};

	static const struct wl_callback_listener wl_surface_frame_listener = {
		.done = surface_frame_done,
	};


	xdg_wm_base_add_listener(display->xdg_wm_base, &xdg_wm_base_listener, display);

	display->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	static const struct xdg_toplevel_listener xdg_toplevel_listener = {
		.configure = xdg_toplevel_configure_handler,
		.close = xdg_toplevel_close_handler,
	};


	xdg_toplevel_add_listener(display->xdg_toplevel, &xdg_toplevel_listener, display);

    if (display->decoration_manager) {
        display->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(display->decoration_manager, display->xdg_toplevel);

		static const struct zxdg_toplevel_decoration_v1_listener xdg_decoration_listener = {
				.configure = xdg_decoration_configure_handler
		};

		zxdg_toplevel_decoration_v1_add_listener(display->decoration, &xdg_decoration_listener, NULL);
	} else {
        display->libdecor = dlopen("libdecor-0.so.0", RTLD_LAZY | RTLD_LOCAL);
        if (display->libdecor) {
            void* ptr = dlsym(display->libdecor, "libdecor_new");
            if (ptr != NULL) memcpy(&libdecor_new, &ptr, sizeof(libdecor_new_type));
            ptr = dlsym(display->libdecor, "libdecor_unref");
            if (ptr != NULL) memcpy(&libdecor_unref, &ptr, sizeof(libdecor_unref_type));

            ptr = dlsym(display->libdecor, "libdecor_decorate");
            if (ptr != NULL) memcpy(&libdecor_decorate , &ptr, sizeof(libdecor_decorate_type));
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

	wl_display_roundtrip(display->dpy);
    wl_registry_destroy(display->registry);

    if (display->decoration_manager != NULL)
		zxdg_decoration_manager_v1_destroy(display->decoration_manager);

    return WAYLIB_TRUE;
}

void wl_surface_frame_done(void* data, struct wl_callback *cb, uint32_t time) {

}

waylib_bool waylib_display_close(waylib_display* display) {
    if (display->libdecor && display->decor_ctx && libdecor_unref) {
        libdecor_unref(display->decor_ctx);
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

waylib_bool waylib_window_init(waylib_display* display, int width, int height, waylib_window* window) {
    waylib_surface_init(display, &window->surface);
    wl_surface_set_user_data(window->surface.surface, window);

	static const struct xdg_surface_listener xdg_surface_listener = {
		.configure = xdg_surface_configure_handler,
	};

	static const struct wl_callback_listener wl_surface_frame_listener = {
		.done = wl_surface_frame_done,
	};

	window->xdg_surface = xdg_wm_base_get_xdg_surface(display->xdg_wm_base, window->surface.surface);
	xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, NULL);
	xdg_surface_set_user_data(window->xdg_surface, window);

	window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
	xdg_toplevel_set_user_data(window->xdg_toplevel, window);

	xdg_surface_set_window_geometry(window->xdg_surface, 0, 0, width, height);

    if (libdecor_new &&  libdecor_unref) {
        static struct libdecor_frame_interface frameInterface = {0}; /*= {
            wl_handle_configure,
            wl_handle_close,
            wl_handle_commit,
            wl_handle_dismiss_popup,
        };*/

        struct libdecor_frame* frame = libdecor_decorate(display->decor_ctx, window->surface.surface, &frameInterface, window);
  //      libdecor_frame_set_app_id(frame, "my-libdecor-app");
//        libdecor_frame_set_title(frame, "My Libdecor Window");
    }

	wl_display_roundtrip(display->dpy);
	wl_surface_commit(window->surface.surface);

	/* wait for the surface to be configured */
	while (wl_display_dispatch(display->dpy) != -1) { }

	struct wl_callback* callback = wl_surface_frame(window->surface.surface);
	wl_callback_add_listener(callback, &wl_surface_frame_listener, window);
	wl_surface_commit(window->surface.surface);
    return WAYLIB_TRUE;
}

waylib_bool waylib_window_free(waylib_window* window) {
    return WAYLIB_TRUE;
}

waylib_bool waylib_surface_init(waylib_display* display, waylib_surface* surface) {
    surface->surface = wl_compositor_create_surface(display->compositor);
    return (surface->surface == NULL);
}

waylib_bool waylib_surface_free(waylib_surface* surface) {
    wl_surface_destroy(surface->surface);
    return WAYLIB_TRUE;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void seat_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities);
struct wl_seat_listener seat_listener = {&seat_capabilities};

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

void registry_add_object(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    waylib_display* display = (waylib_display*)data;
    if (!strcmp(interface, wl_compositor_interface.name)) {
        display->compositor = (struct wl_compositor*)(wl_registry_bind (registry, name, &wl_compositor_interface, 1));
    } else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
        display->subcompositor = (struct wl_subcompositor*)(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        display->seat = (struct wl_seat*)(wl_registry_bind (registry, name, &wl_seat_interface, 0));
        wl_seat_add_listener (display->seat, &seat_listener, data);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        display->shm = (struct wl_shm*)(wl_registry_bind(registry, name, &wl_shm_interface, 1));
        display->cursor_theme = wl_cursor_theme_load(NULL, 32, display->shm);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        display->xdg_wm_base = (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, MIN(version, 1));
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        display->decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

void registry_remove_object(void *data, struct wl_registry *registry, uint32_t name) {
	/* do nothing... */
}

#endif /* WAYLIB_C */
