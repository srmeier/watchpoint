# NOTE: For commands but no debugger
# make
# UNFLoader -r watchpoint.z64 -d -p

# NOTE: For commands with debugger
# make D=1
# UNFLoader -r watchpoint.z64 -g -p

BUILD_DIR=build
include $(N64_INST)/include/n64.mk

src = main.c debug.c

ifeq ($(D),1)
N64_CFLAGS += -g -O0 -D USE_RDBTHREAD=1 -D DEBUG_RDP=0
N64_LDFLAGS += -g
endif

assets_png = $(wildcard assets/*.png)
assets_conv = $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite)))

MKSPRITE_FLAGS ?=

all: watchpoint.z64

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	@$(N64_MKSPRITE) -f RGBA16 --compress -o "$(dir $@)" "$<"

$(BUILD_DIR)/watchpoint.dfs: $(assets_conv)
$(BUILD_DIR)/watchpoint.elf: $(src:%.c=$(BUILD_DIR)/%.o)

watchpoint.z64: N64_ROM_TITLE="Watchpoint"
watchpoint.z64: $(BUILD_DIR)/watchpoint.dfs

clean:
	rm -rf $(BUILD_DIR) *.z64
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean
