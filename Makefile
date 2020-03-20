
Q	= @

BUILD_DIR	= $(CURDIR)/build
SRC_DIR		= $(CURDIR)/src

CFLAGS_STD	= -std=gnu17
CFLAGS_W	= -Wall -Wextra -Werror
CFLAGS_O	= -O3 -march=native -flto -fuse-linker-plugin
CFLAGS_PKG	= `pkg-config --cflags libalx-base`
CFLAGS_PKG	+= `pkg-config --cflags libalx-cv`
CFLAGS		= $(CFLAGS_W) $(CFLAGS_O) $(CFLAGS_PKG)

LIBS		= `pkg-config --libs libalx-cv`
LIBS		+= `pkg-config --libs libalx-base`




################################################################################
# dependencies

OBJ	=								\
	$(BUILD_DIR)/label.o						\
	$(BUILD_DIR)/symbols.o						\
	$(BUILD_DIR)/templates.o					\
	$(BUILD_DIR)/main.o

SRC	=								\
	$(SRC_DIR)/label.c						\
	$(SRC_DIR)/symbols.c						\
	$(SRC_DIR)/templates.c						\
	$(SRC_DIR)/main.c

DEP	= $(OBJ:.o=.d)

.PHONY: all
all: wash
	@:

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	$(Q)mkdir -p		$(@D)/
	@echo	"	CC -M	build/tmp/base/stdio/$*.d"
	$(Q)$(CC) $(CFLAGS) -MG -MT"$@" -MT"$(BUILD_DIR)/$*.s" -M $< -MF $@
$(BUILD_DIR)/%.s: $(SRC_DIR)/%.c $(BUILD_DIR)/%.d
	$(Q)mkdir -p		$(@D)/
	@echo	"	CC	$*.s"
	$(Q)$(CC) $(CFLAGS) -S $< -o $@
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	@echo	"	AS	$*.o"
	$(Q)$(AS) $< -o $@

wash: $(OBJ)
	@echo	"	CC	$@"
	$(Q)gcc $(CFLAGS) $^ -o $@ $(LIBS)

include $(DEP)

.PHONY: clean
clean:
	rm -rf build/
