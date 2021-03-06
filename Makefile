TARGET_STRING := n64game
TARGET := $(TARGET_STRING)

# Preprocessor definitions
DEFINES := _FINALROM=1 NDEBUG=1 F3DEX_GBI_2=1

SRC_DIRS :=
USE_DEBUG := 0

TOOLS_DIR := tools

# Whether to hide commands or not
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
  V := @
endif

# Whether to colorize build messages
COLOR ?= 1

ifeq ($(filter clean distclean print-%,$(MAKECMDGOALS)),)
  # Make tools if out of date
  $(info Building tools...)
  DUMMY != $(MAKE) -s -C $(TOOLS_DIR) >&2 || echo FAIL
    ifeq ($(DUMMY),FAIL)
      $(error Failed to build tools)
    endif
  $(info Building ROM...)
endif

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR := build
ROM            := $(TARGET_STRING).z64
ELF            := $(BUILD_DIR)/$(TARGET_STRING).elf
LD_SCRIPT      := $(TARGET_STRING).ld
CODESEGMENT	   := $(BUILD_DIR)/codesegment.o
BOOT		:= /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	:= $(BUILD_DIR)/boot.6102.o

# Directories containing source files
SRC_DIRS += src asm

C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

SRC_OBJECTS := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
			$(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o))
			
# Object files
O_FILES := $(SRC_OBJECTS) \
		   $(BOOT_OBJ) 

#Filesystem data
FILESYSTEM := $(BUILD_DIR)/filesystem.bin
FILESYSTEM_HEADER := $(BUILD_DIR)/filesystem.fsh
FILESYSTEM_ROOT := data

GFX_DIRS := gfx
GFX_FILES := $(foreach dir,$(GFX_DIRS),$(wildcard $(dir)/*.png))

GFX_OUTFILES := $(foreach file,$(GFX_FILES),$(FILESYSTEM_ROOT)/$(file:.png=.img))

# Automatic dependency files
DEP_FILES := $(SRC_OBJECTS:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

AS        := mips-n64-as
CC        := mips-n64-gcc
CPP       := cpp
LD        := mips-n64-ld
AR        := mips-n64-ar
OBJDUMP   := mips-n64-objdump
OBJCOPY   := mips-n64-objcopy

INCLUDE_DIRS += /usr/include/n64 /usr/include/n64/nusys include $(BUILD_DIR) src asm .

C_DEFINES := $(foreach d,$(DEFINES),-D$(d))
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

CFLAGS = -Werror=implicit-function-declaration -ffunction-sections -fdata-sections -mdivide-breaks -G 0 -Os -mabi=32 -ffreestanding -mfix4300 $(DEF_INC_CFLAGS)
ASFLAGS     := -march=vr4300 -mabi=32 $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(foreach d,$(DEFINES),--defsym $(d))

# C preprocessor flags
CPPFLAGS := -P -Wno-trigraphs $(DEF_INC_CFLAGS)

# tools
MAKEFS := $(TOOLS_DIR)/makefs
MAKEIMAGE := $(TOOLS_DIR)/makeimage
PRINT = printf

ifeq ($(COLOR),1)
NO_COL  := \033[0m
RED     := \033[0;31m
GREEN   := \033[0;32m
BLUE    := \033[0;34m
YELLOW  := \033[0;33m
BLINK   := \033[33;5m
endif

# Common build print status function
define print
  @$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

# Default target
default: $(ROM)

clean:
	$(RM) -r $(BUILD_DIR) $(addprefix $(FILESYSTEM_ROOT)/,$(GFX_DIRS))

distclean: clean
	$(MAKE) -C $(TOOLS_DIR) clean

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS)) $(addprefix $(FILESYSTEM_ROOT)/,$(GFX_DIRS))

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

$(BUILD_DIR)/asm/bin_data.o: $(FILESYSTEM) $(FILESYSTEM_HEADER)

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<
	
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s
	$(call print,Assembling:,$<,$@)
	$(V)$(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d  -o $@ $<

# Run linker script through the C preprocessor
$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT)
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	$(V)$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# Link final ELF file
$(ELF): $(O_FILES) $(BUILD_DIR)/$(LD_SCRIPT)
	@$(PRINT) "$(GREEN)Linking ELF file:  $(BLUE)$@ $(NO_COL)\n"
	$(V)$(LD) -L $(BUILD_DIR) -T $(BUILD_DIR)/$(LD_SCRIPT) -Map $(BUILD_DIR)/$(TARGET_STRING).map --gc-sections --no-check-sections -o $@ $(O_FILES) -L/usr/lib/n64/nusys -lnusys -lnualstl -L/usr/lib/n64 -lmus -lultra_rom -L$(N64_LIBGCCDIR) -lgcc

# Build ROM
$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	$(V)makemask $@

#Generate filesystem
$(FILESYSTEM): $(GFX_OUTFILES)
	@$(PRINT) "$(GREEN)Creating filesystem: $(BLUE)$@ $(NO_COL)\n"
	$(V)$(MAKEFS) $(FILESYSTEM_ROOT) $(FILESYSTEM_HEADER) $(FILESYSTEM)
	
#Generate graphics
data/%.img: %.png
	$(call print,Converting image:,$<,$@)
	$(V)$(MAKEIMAGE) $< $@

#Force build filesystem every time
.PHONY: $(FILESYSTEM)

.PHONY: clean distclean default
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true