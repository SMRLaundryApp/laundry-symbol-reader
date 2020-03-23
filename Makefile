#! /usr/bin/make -f

################################################################################
################################################################################
# Beautify output
Q	= @

################################################################################
# directories

SRC_DIR		= $(CURDIR)/src
SHARE_DIR	= $(CURDIR)/share
BUILD_DIR	= $(CURDIR)/build

INSTALL_BIN_DIR		= /usr/local/bin
INSTALL_SHARE_DIR	= /usr/local/share

################################################################################
# Make variables (CC, etc...)
  CC	= gcc
  AS	= as
  SZ	= size

################################################################################
# cflags
CFLAGS_STD	= -std=gnu17
CFLAGS_W	= -Wall -Wextra -Wno-format -Werror
CFLAGS_O	= -O3 -march=native -flto
CFLAGS_PKG	= `pkg-config --cflags libalx-base`
CFLAGS_PKG	+= `pkg-config --cflags libalx-cv`
CFLAGS		= $(CFLAGS_W) $(CFLAGS_O) $(CFLAGS_PKG)

################################################################################
# libs
LIBS	= -flto
LIBS  	+= -fuse-linker-plugin
LIBS    += `pkg-config --libs libalx-cv`
LIBS	+= `pkg-config --libs libalx-base`




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

################################################################################
# compile
.PHONY: all
all: $(BUILD_DIR)/laundry-symbol-reader

$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c
	$(Q)mkdir -p		$(@D)/
	@echo	"	CC -M	$*.d"
	$(Q)$(CC) $(CFLAGS) -MG -MT"$@" -MT"$(BUILD_DIR)/$*.s" -M $< -MF $@
$(BUILD_DIR)/%.s: $(SRC_DIR)/%.c $(BUILD_DIR)/%.d
	$(Q)mkdir -p		$(@D)/
	@echo	"	CC	$*.s"
	$(Q)$(CC) $(CFLAGS) -S $< -o $@
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	@echo	"	AS	$*.o"
	$(Q)$(AS) $< -o $@

$(BUILD_DIR)/laundry-symbol-reader: $(OBJ)
	@echo	"	CC	$*"
	$(Q)gcc $(CFLAGS) $^ -o $@ $(LIBS)

include $(DEP)

################################################################################
# install




.PHONY: install
install: | inst-bin
install: | inst-share


.PHONY: inst-bin
inst-bin:
	$(Q)mkdir -p		$(DESTDIR)/$(INSTALL_BIN_DIR)/
	@echo	"	CP -f	$(DESTDIR)/$(INSTALL_BIN_DIR)/laundry-symbol-reader"
	$(Q)cp  -f $(v)		$(BUILD_DIR)/laundry-symbol-reader	\
					$(DESTDIR)/$(INSTALL_BIN_DIR)/

.PHONY: inst-share
inst-share:
	$(Q)mkdir -p		$(DESTDIR)/$(INSTALL_SHARE_DIR)/laundry-symbol-reader/
	@echo	"	CP -rf	$(DESTDIR)/$(INSTALL_SHARE_DIR)/laundry-symbol-reader/*"
	$(Q)cp -r -f $(v)	$(SHARE_DIR)/*				\
					$(DESTDIR)/$(INSTALL_SHARE_DIR)/laundry-symbol-reader/


################################################################################
# uninstall
.PHONY: uninstall
uninstall:
	@echo	"	Uninstall:"
	@echo	"	RM -f	$(DESTDIR)/$(INSTALL_BIN_DIR)/laundry-symbol-reader"
	$(Q)rm -f $(v)		$(DESTDIR)/$(INSTALL_BIN_DIR)/laundry-symbol-reader
	@echo	"	RM -rf	$(DESTDIR)/$(INSTALL_SHARE_DIR)/laundry-symbol-reader/"
	$(Q)rm -f -r $(v)	$(DESTDIR)/$(INSTALL_SHARE_DIR)/laundry-symbol-reader/
	@echo	"	Done"


################################################################################
# clean
.PHONY: clean
clean:
	@echo	"	RM	$(BUILD_DIR)"
	$(Q)rm -rf $(v)		$(BUILD_DIR)


################################################################################
######## End of file ###########################################################
################################################################################

