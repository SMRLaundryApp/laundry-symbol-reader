#! /usr/bin/make -f

################################################################################
# dependencies

MK_DEPS	=								\
	$(MK_DIR)/Makefile						\
	$(MAIN_DIR)/Makefile

MODULES	=								\
	label								\
	main								\
	symbols								\
	templates/base							\
	templates/templates

SRC	= $(MODULES:%=$(SRC_DIR)/%.c)
OBJ	= $(MODULES:%=$(BUILD_DIR)/%.o)
DEP	= $(OBJ:.o=.d)

################################################################################
# target: dependencies
#	action

PHONY := all
all: $(BUILD_DIR)/laundry-symbol-reader
	@:

$(BUILD_DIR)/laundry-symbol-reader: $(OBJ)
	@echo	"	CC	$(@F)"
	$(Q)$(CC) $(CFLAGS) $^ -o $@ $(LIBS)



$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c $(MK_DEPS)
	$(Q)mkdir -p		$(@D)/
	@echo	"	CC -M	$*.d"
	$(Q)$(CC) $(CFLAGS) -I $(SRC_DIR)				\
			-MG -MT"$@" -MT"$(BUILD_DIR)/$*.s" -M $< -MF $@
$(BUILD_DIR)/%.s: $(SRC_DIR)/%.c $(BUILD_DIR)/%.d
	@echo	"	CC	$*.s"
	$(Q)$(CC) $(CFLAGS) -I $(SRC_DIR) -S $< -o $@
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	@echo	"	AS	$*.o"
	$(Q)$(AS) $< -o $@

include $(DEP)


PHONY += clean
clean:
	$(Q)rm -f *.o *.s

################################################################################
# Declare the contents of the .PHONY variable as phony.
.PHONY: $(PHONY)


################################################################################
######## End of file ###########################################################
################################################################################

