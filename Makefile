EXAMPLES_SOURCES = $(wildcard examples/*.c)
EXAMPLES = $(patsubst examples/%.c, build/%, $(EXAMPLES_SOURCES))
OUTDIR = build

all: gen_headers $(EXAMPLES)

$(OUTDIR):
	mkdir -p $(OUTDIR)

LIBS = -lwayland-cursor -lwayland-client -lxkbcommon  -lwayland-egl

$(EXAMPLES): $(OUTDIR)/%: examples/%.c $(OUTDIR) waylib.c waylib.h
	$(CC) -o $@ $< -I./ $(LIBS) -g3

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
	@wayland-scanner private-code /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml xdg-decoration-unstable-v1.c
	@wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg-shell-client-protocol.c
