PROGNAME := easydevelop
SRCDIR   := src
BUILDDIR := build
RESDIR   := res

CFLAGS ?= -O2
CFLAGS += -Wall -Wextra -pedantic

CPPFLAGS ?=
CPPFLAGS += -DPROGNAME=\"$(PROGNAME)\" -DRESDIR=\"$(PWD)/$(RESDIR)\"

LDFLAGS ?= -s

CFLAGS_sdl != pkg-config --cflags sdl2 SDL2_ttf
LDLIBS_sdl != pkg-config --libs sdl2 SDL2_ttf
CFLAGS     += $(CFLAGS_sdl)
LDLIBS     += $(LDLIBS_sdl)

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all
all: $(PROGNAME)

.PHONY: clean
clean:
	rm -f $(PROGNAME)
	rm -rf $(BUILDDIR)

$(PROGNAME): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $@
