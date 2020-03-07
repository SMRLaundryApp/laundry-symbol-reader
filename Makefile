
Q	= @

CFLAGS_STD	= -std=gnu17
CFLAGS_W	= -Wall -Wextra -Werror
CFLAGS_O	= -O3 -march=native -flto -fuse-linker-plugin
CFLAGS_PKG	= `pkg-config --cflags libalx-base`
CFLAGS_PKG	+= `pkg-config --cflags libalx-cv`
CFLAGS		= $(CFLAGS_W) $(CFLAGS_O) $(CFLAGS_PKG)

LIBS		= `pkg-config --libs libalx-cv`
LIBS		+= `pkg-config --libs libalx-base`

.PHONY: all
all: wash

wash: main.c
	@echo	"	CC	$@"
	$(Q)gcc $(CFLAGS) $< -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -f wash
