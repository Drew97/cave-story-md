MARSDEV ?= ${HOME}/mars
MARSBIN  = $(MARSDEV)/m68k-elf/bin
TOOLSBIN = $(MARSDEV)/bin

CC   = $(MARSBIN)/m68k-elf-gcc
AS   = $(MARSBIN)/m68k-elf-as
LD   = $(MARSBIN)/m68k-elf-ld
NM   = $(MARSBIN)/m68k-elf-nm
OBJC = $(MARSBIN)/m68k-elf-objcopy

ASMZ80   = $(TOOLSBIN)/sjasm
BINTOS   = $(TOOLSBIN)/bintos
RESCOMP  = $(TOOLSBIN)/rescomp
XGMTOOL  = $(TOOLSBIN)/xgmtool
WAVTORAW = $(TOOLSBIN)/wavtoraw

GCC_VER := $(shell $(CC) -dumpversion)
PLUGIN   = $(MARSDEV)/m68k-elf/libexec/gcc/m68k-elf/$(GCC_VER)
LTO_SO   = liblto_plugin.so
ifeq ($(OS),Windows_NT)
	LTO_SO = liblto_plugin-0.dll
endif

INCS     = -Isrc -Ires -Iinc
LIBS     = -L$(MARSDEV)/m68k-elf/lib/gcc/m68k-elf/$(GCC_VER) -lgcc
CCFLAGS  = -m68000 -Wall -Wextra -std=c99 -fno-builtin -fshort-enums
OPTIONS  = 
ASFLAGS  = -m68000 --register-prefix-optional
LDFLAGS  = -T $(MARSDEV)/ldscripts/sgdk.ld -nostdlib
Z80FLAGS = -isrc/xgm

BOOTSS    = $(wildcard src/boot/*.s)
BOOT_OBJS = $(BOOTSS:.s=.o)

RESS  = res/resources.res
Z80S  = $(wildcard src/xgm/*.s80)
CS    = $(wildcard src/*.c)
CS   += $(wildcard src/ai/*.c)
CS   += $(wildcard src/db/*.c)
CS   += $(wildcard src/xgm/*.c)
SS    = $(wildcard src/*.s)
SS   += $(wildcard src/xgm/*.s)
OBJS  = $(RESS:.res=.o)
OBJS += $(Z80S:.s80=.o)
OBJS += $(CS:.c=.o)
OBJS += $(SS:.s=.o)

.SECONDARY: doukutsu.elf

.PHONY: all ntsc ntsc-debug pal pal-debug

all: ntsc

ntsc: release
ntsc-debug: debug

pal: CCFLAGS += -DPAL
pal: release
pal-debug: CCFLAGS += -DPAL
pal-debug: debug

.PHONY: release debug main-build

release: OPTIONS  = -O3 -fno-web -fno-gcse -fno-unit-at-a-time -fomit-frame-pointer
release: OPTIONS += -flto -fuse-linker-plugin
release: main-build symbol.txt

# Gens-KMod, BlastEm and UMDK support GDB tracing, enabled by this target
debug: OPTIONS = -g -O2 -DDEBUG -DKDEBUG
debug: main-build symbol.txt

main-build: head-gen doukutsu.bin

# Cross reference symbol.txt with the addresses displayed in the crash handler
symbol.txt: doukutsu.bin
	$(NM) --plugin=$(PLUGIN)/$(LTO_SO) -n doukutsu.elf > symbol.txt

src/boot/sega.o: src/boot/rom_head.bin
	$(AS) $(ASFLAGS) src/boot/sega.s -o $@

%.bin: %.elf
	$(OBJC) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=8K conv=sync

%.elf: $(BOOT_OBJS) $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(BOOT_OBJS) $(OBJS) $(LIBS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CCFLAGS) $(OPTIONS) $(INCS) -c $< -o $@

%.o: %.s 
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@

%.s: %.res
	$(RESCOMP) $< $@

%.o80: %.s80
	$(ASMZ80) $(Z80FLAGS) $< $@ out.lst

%.s: %.o80
	$(BINTOS) $<

src/boot/rom_head.o: src/boot/rom_head.c
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

src/boot/rom_head.bin: src/boot/rom_head.o
	$(LD) $(LDFLAGS) --oformat binary $< -o $@


.PHONY: head-gen clean

head-gen:
	rm -f inc/ai_gen.h
	python aigen.py

clean:
	rm -f $(OBJS)
	rm -f doukutsu.bin doukutsu.elf temp.bin symbol.txt
	rm -f src/boot/sega.o src/boot/rom_head.o src/boot/rom_head.bin
	rm -f src/xgm/z80_xgm.s src/xgm/z80_xgm.o80 src/xgm/z80_xgm.h out.lst
	rm -f res/resources.h res/resources.s
	rm -f inc/ai_gen.h
