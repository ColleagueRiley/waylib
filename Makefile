EXAMPLES_SOURCES = $(wildcard examples/*.c)
EXAMPLES = $(patsubst examples/%.c, build/%, $(EXAMPLES_SOURCES))
OUTDIR = build

all: gen_headers build/libwaylib.a build/libwaylib.so $(EXAMPLES)

$(OUTDIR):
	mkdir -p $(OUTDIR)

LIBS = -lwayland-cursor -lwayland-client -lxkbcommon  -lwayland-egl

$(EXAMPLES): $(OUTDIR)/%: examples/%.c $(OUTDIR) waylib.c waylib.h
	$(CC) -o $@ $< -I./  $(LIBS) -g3

build/waylib.o: waylib.c waylib.h $(OUTDIR)
	$(CC) -c -fPIC $< -o $@

build/libwaylib.so: build/waylib.o waylib.c waylib.h
	$(CC) -shared $< $(LIBS) -o $@

build/libwaylib.a: build/waylib.o waylib.c waylib.h
	ar rcs $@ $<

debug: $(EXAMPLES)
	@for exe in $(EXAMPLES); do \
		echo "Running $$exe..."; \
		./$$exe$(EXT); \
	done

clean:
	rm -r -f build

gen_headers:
	@wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.h
	@wayland-scanner client-header /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml xdg-decoration-unstable-v1.h
	@wayland-scanner client-header /usr/share/wayland-protocols/stable/viewporter/viewporter.xml viewporter-client-protocol.h
	@wayland-scanner private-code /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml xdg-decoration-unstable-v1.c
	@wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c
	@wayland-scanner private-code /usr/share/wayland-protocols/stable/viewporter/viewporter.xml viewporter-client-protocol.c
