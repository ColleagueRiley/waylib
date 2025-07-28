# Waylib
Waylib A portable "high level" low-level Wayland client API.

The Wayland API sucks, in part because of how low level it is, you have to manually define 1000 things to create a simple window that does nothing.
But also because Wayland is so fragmented and certain aspects may or may not exist depending on the compositor.

As much as I dislike Wayland, it is clear that the FOSS masses don't care, so it will inevitably replace X11.

I am creating this API so that way future devs working with the low level Wayland API can use or take from this rather than having with Wayland's API alone.\
You can think of it as "Xlib but for Wayland" in terms of XLib being a 'high level API' compared to dealing with XCB or the X protocol directly.

There is [another project](https://github.com/vioken/waylib) called Waylib, it's "A wrapper for wlroots based on Qt" but I thought the name was funny before I knew about that project. So I'm using it anyway.

# compiling and linking
waylib is made up of two files `waylib.h` and `waylib.c`, you are free to compile and link waylib however you want.

The makefile includes a recipe for compiling `waylib.c` into `libwaylib.so`, `waylib.o` or `libwaylib.a`  but I prefer using it like a single-header file:

```c
#define WAYLIB_API inline
#include "waylib.h"
#include "waylib.c"
```
