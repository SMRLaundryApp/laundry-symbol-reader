#! /usr/bin/make -f

################################################################################
################################################################################
# Beautify output
Q	= @

export	Q

################################################################################
# Do not print "Entering directory ..."
MAKEFLAGS += --no-print-directory

################################################################################
# directories

MAIN_DIR	= $(CURDIR)

BIN_DIR		= $(CURDIR)/bin
BUILD_DIR	= $(CURDIR)/build
MK_DIR		= $(CURDIR)/mk
SRC_DIR		= $(CURDIR)/src
SHARE_DIR	= $(CURDIR)/share

INSTALL_BIN_DIR		= /usr/local/bin
INSTALL_SHARE_DIR	= /usr/local/share

export	MAIN_DIR

export	BUILD_DIR
export	MK_DIR
export	SRC_DIR

################################################################################
# Make variables (CC, etc...)
  CC	= gcc
  AS	= as
  SZ	= size

export	CC
export	AS
export	SZ

################################################################################
# cflags
CFLAGS_STD	= -std=gnu17
CFLAGS_W	= -Wall -Wextra -Wno-format -Werror
CFLAGS_O	= -O3 -march=native -flto
CFLAGS_PKG	= `pkg-config --cflags libalx-base`
CFLAGS_PKG	+= `pkg-config --cflags libalx-cv`
CFLAGS		= $(CFLAGS_W) $(CFLAGS_O) $(CFLAGS_PKG)

export	CFLAGS

################################################################################
# libs
LIBS	= -flto
LIBS  	+= -fuse-linker-plugin
LIBS    += `pkg-config --libs libalx-cv`
LIBS	+= `pkg-config --libs libalx-base`

export	LIBS

################################################################################
# compile
.PHONY: all
all:
	$(Q)$(MAKE)	-C $(MK_DIR)

################################################################################
# install




.PHONY: install
install: | inst-bin
install: | inst-share
install: | inst-scripts

.PHONY: inst-scripts
inst-scripts:
	$(Q)chmod +x		$(BIN_DIR)/*
	$(Q)mkdir -p		$(DESTDIR)/$(INSTALL_BIN_DIR)/
	@echo	"	CP -rf	$(DESTDIR)/$(INSTALL_BIN_DIR)/*"
	$(Q)cp  -r -f $(v)	$(BIN_DIR)/*				\
					$(DESTDIR)/$(INSTALL_BIN_DIR)/

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

