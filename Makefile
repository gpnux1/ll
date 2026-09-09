include config.mk
# config.mk 之后的命令行覆盖: DEBUG=1 时产物改名 (build_debug/ll_debug.*)

ifeq ($(DEBUG),1)
BUILD_NAME := ll_debug
endif

MAKEFLAGS += --no-print-directory

.SUFFIXES:
.SECONDARY:
.DELETE_ON_ERROR:
.SECONDEXPANSION:

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

ifeq ($(OS),Windows_NT)
EXE := .exe
else
EXE :=
endif

TOOLS_DIR 		:= $(ROOT_DIR)/tools
FIX       		:= $(TOOLS_DIR)/gbafix/gbafix$(EXE)
PREPROCFLAGS	:= charmap.txt
PREPROC			:= $(TOOLS_DIR)/preproc/preproc$(EXE)

SHELL  := /bin/bash -o pipefail
SHA1   := $(shell { command -v sha1sum || command -v shasum; } 2>/dev/null) -c

### TOOLCHAIN ###

PREFIX  := arm-none-eabi-
CC1     := tools/agbcc/bin/agbcc$(EXE)
CC1_OLD := tools/agbcc/bin/old_agbcc$(EXE)
CPP     := $(PREFIX)cpp
LD      := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
AS      := $(PREFIX)as

### FILES ###

OBJ_DIR := build
ifeq ($(DEBUG),1)
# 调试构建 (-O1): .text 膨胀, 独立产物目录/名字 + 放松 ROM 锚点的链接脚本, 不跑 SHA1
OBJ_DIR    := build_debug
endif
ROM     := $(BUILD_NAME).gba
ELF     := $(BUILD_NAME).elf
MAP     := $(BUILD_NAME).map

ASM_SUBDIR   := asm
ASM_BUILDDIR := $(OBJ_DIR)/$(ASM_SUBDIR)

C_SUBDIR   := src
C_BUILDDIR := $(OBJ_DIR)/$(C_SUBDIR)

DATA_SUBDIR   := data
DATA_BUILDDIR := $(OBJ_DIR)/$(DATA_SUBDIR)

ASM_SRCS := $(wildcard $(ASM_SUBDIR)/*.s)
ASM_OBJS := $(patsubst $(ASM_SUBDIR)/%.s,$(ASM_BUILDDIR)/%.o,$(ASM_SRCS))

C_SRCS := $(wildcard $(C_SUBDIR)/*.c)
C_OBJS := $(patsubst $(C_SUBDIR)/%.c,$(C_BUILDDIR)/%.o,$(C_SRCS))

DATA_SRCS := $(wildcard $(DATA_SUBDIR)/*.s)
DATA_OBJS := $(patsubst $(DATA_SUBDIR)/%.s,$(DATA_BUILDDIR)/%.o,$(DATA_SRCS))

OBJS     := $(C_OBJS) $(ASM_OBJS) $(DATA_OBJS)
OBJS_REL := $(patsubst $(OBJ_DIR)/%,%,$(OBJS))

### FLAGS ###

ASFLAGS  := -mcpu=arm7tdmi -mthumb-interwork  -I sound
CPPFLAGS := -nostdinc -I tools/agbcc/include -iquote include
CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -O2 -g -fhex-asm -fprologue-bugfix

LDSCRIPT    := linker.ld

ifeq ($(DEBUG),1)
# 调试构建: .text 总量必须 ≤ 8MB (ROM 上限) — -O0 膨胀 +68KB 会溢出 75KB (数据 blob
# 为原 ROM 固定内容无法压缩), 故用 -O1 (实测 script_vm.o: O2=16752 / O1=16808 / O0=22828)
CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -O1 -g -fhex-asm -fprologue-bugfix
LDSCRIPT  := linker_debug.ld
endif

# LIBS := tools/agbcc/lib/libgcc.a tools/agbcc/lib/libc.a
LIBPATH := -L $(ROOT_DIR)/tools/agbcc/lib
LIBS := $(LIBPATH) -lgcc -lc

### FORMATTER ###

FORMAT := clang-format
FORMAT_SRCS := $(shell find src include -name "*.c" -o -name "*.h")

### CONTEXT ###

C_HEADERS := $(shell find include -name "*.h" -not -name "include_asm.h")
CONTEXT_FLAGS := -DM2C -DPLATFORM_GBA=1 -Dsize_t=int

### TARGETS ###

.PHONY: all rom compare progress clean tidy format check_format ctx code.s asm verify remaining

$(shell mkdir -p $(ASM_BUILDDIR) $(C_BUILDDIR) $(DATA_BUILDDIR))

ifeq ($(DEBUG),1)
# fix_debug_rom 需要 ll.elf (正式版符号表=旧地址来源): 先隐式构建正式版
all: rom
	@$(MAKE) DEBUG= BUILD_NAME=ll OBJ_DIR=build rom >/dev/null
	@python3 scripts/fix_debug_rom.py
else
all: compare
endif

compare: rom
	$(SHA1) $(BUILD_NAME).sha1
	@$(MAKE) progress

# 匹配进度: functions.tsv status=1 占比 (独立运行: make progress)
progress:
	@awk -F'\t' 'NR>1 && NF>1 {t++; if ($$1==1) m++} END {printf "匹配进度: %d/%d (%.1f%%)\n", m, t, m*100/t}' functions.tsv

rom: $(ROM)

rom: $(ROM)

# DEBUG 构建链接脚本: 由 gen_debug_ld.py 从 linker.ld 生成
# ① rom region 8M->16M; ② .text 段剔除 GAP 名单文件 + 尾部补洞锚 (保数据起点
#   0x0805769C 不漂移); ③ GAP 文件整体搬 0x08800000 空区; ④ 数据区与锚 A/B 原样保留
# 数据区内指向 GAP 函数的指针由 fix_debug_rom.py 链接后修补 (见 DEBUG 修补规则)
DBG_GAP ?= event_hub event_actor
linker_debug.ld: linker.ld scripts/gen_debug_ld.py
	@python3 scripts/gen_debug_ld.py --gap "$(DBG_GAP)"

$(ELF): $(OBJS) $(LDSCRIPT)
	@echo "$(LD) -T $(LDSCRIPT) -Map $(MAP) <objects> -o $@"
	@cd $(OBJ_DIR) && $(LD) -T ../$(LDSCRIPT) -Map ../$(MAP) $(OBJS_REL) $(LIBS) -o ../$(ELF)
	$(FIX) $@ -t"$(TITLE)" -c$(GAME_CODE) -m$(MAKER_CODE) -r$(REVISION) --silent

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@

### RECIPES ###

# Assemble standalone .s files (crt0, libgcc, rom_header)
$(ASM_BUILDDIR)/%.o: $(ASM_SUBDIR)/%.s
	@echo "$(AS) <flags> -o $@ $<"
	@$(AS) $(ASFLAGS) -o $@ $<

$(C_BUILDDIR)/m4a_tables.o: CC1 := $(TOOLS_DIR)/agbcc/bin/old_agbcc
$(C_BUILDDIR)/m4a_tables.o: CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -Werror -O2 -g
$(C_BUILDDIR)/m4a.o: CC1 := $(TOOLS_DIR)/agbcc/bin/old_agbcc
$(C_BUILDDIR)/m4a.o: CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -Werror -O2 -g
$(C_BUILDDIR)/agb_sram.o: CC1 := $(TOOLS_DIR)/agbcc/bin/old_agbcc
$(C_BUILDDIR)/agb_sram.o: CC1FLAGS := -mthumb-interwork -Wimplicit -Wparentheses -Werror -O1 -g

# INCLUDE_ASM 展开成汇编期的 `.include "asm/<dir>/<func>.s"`, make 看不到这层依赖。
# 不补就会踩到: 改 ll.cfg 函数名 + 重生成 code.s + split_asm 后, 引用该函数的
# 其它 C 文件 的 .o **不会重编**, 链接期报 undefined reference 旧名 (已实际踩到)。
# 在解析期用 grep 把依赖补上; 用 $(wildcard) 过滤, 避开注释里那些已不存在的 .s。
define ADD_ASM_DEPS
$(C_BUILDDIR)/$(1).o: $$(filter $$(wildcard $(ASM_SUBDIR)/*/*.s),\
  $$(shell grep -oE 'INCLUDE_ASM\("$(ASM_SUBDIR)/[a-z]+", *[A-Za-z0-9_]+\)' $(C_SUBDIR)/$(1).c 2>/dev/null \
    | sed -E 's|INCLUDE_ASM\("$(ASM_SUBDIR)/([a-z]+)", *([A-Za-z0-9_]+)\)|$(ASM_SUBDIR)/\1/\2.s|'))
endef
$(foreach c,$(notdir $(basename $(C_SRCS))),$(eval $(call ADD_ASM_DEPS,$c)))

# Compile C files (with INCLUDE_ASM support)
$(C_BUILDDIR)/%.o: $(C_SUBDIR)/%.c
	@echo "$(CC1) <flags> -o $@ $<"
	@$(CPP) $(CPPFLAGS) $< -o $(C_BUILDDIR)/$*.i
	@$(PREPROC) $(C_BUILDDIR)/$*.i | $(CC1) $(CC1FLAGS) -o $(C_BUILDDIR)/$*.s
# 	@$(CC1) $(CC1FLAGS) -o $(C_BUILDDIR)/$*.s $(C_BUILDDIR)/$*.i
	@printf ".text\n\t.align\t2, 0\n" >> $(C_BUILDDIR)/$*.s
	@$(AS) $(ASFLAGS) -o $@ $(C_BUILDDIR)/$*.s

# Assemble data files
$(DATA_BUILDDIR)/%.o: $(DATA_SUBDIR)/%.s
	@echo "$(AS) <flags> -o $@ $<"
	@$(AS) $(ASFLAGS) -o $@ $<

ctx.c: $(C_HEADERS)
	@for header in $(C_HEADERS); do echo "#include \"$$header\""; done > ctx.h
	@gcc -P -E -dD -undef -I tools/agbcc/include -I include $(CONTEXT_FLAGS) ctx.h \
		| sed '/#undef/d' \
		| sed '/typedef unsigned long int int;/d' \
		| sed 's/__attribute__((.*))//' \
		| sed '/^#define __STDC/d' \
		| sed '/^#define __GCC/d' \
		| sed '/^#define GUARD_/d' \
		> ctx.c
	@rm ctx.h
	@echo "Generated ctx.c ($$(wc -l < ctx.c) lines)"

ctx: ctx.c

# ==== 重构 R5: 工作流目标 ====
# 重出反汇编 (gbadisasm, 改名管线第 2 步)
code.s:
	tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > code.s

# 从 TSV+ll.cfg+code.s 增量重建 asm/ (内容不变不 touch)
asm:
	python3 scripts/gen_asm.py

# 全量终验: make + SHA1 + audit + status=1 全量字节核验
verify:
	timeout 900 make > /dev/null 2>&1 && sha1sum -c ll.sha1
	python3 scripts/audit.py
	python3 scripts/verify_all.py

# 剩余报表: 还剩哪些未匹配 (MODULE=xxx 过滤)
remaining:
	python3 scripts/remaining.py


format:
	$(FORMAT) -i -style=file $(FORMAT_SRCS)

check_format:
	$(FORMAT) -style=file --dry-run --Werror $(FORMAT_SRCS)

clean: tidy

tidy:
	$(RM) -r build build_debug
	$(RM) $(BUILD_NAME).gba $(BUILD_NAME).elf $(BUILD_NAME).map
	$(RM) ll.gba ll.elf ll.map ll_debug.gba ll_debug.elf ll_debug.map
