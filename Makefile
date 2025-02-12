# run `make all` to compile the .hhk and .bin file, use `make` to compile only the .bin file.
# The .hhk file is the original format, the bin file is a newer format.
AS:=sh4aeb-elf-gcc
AS_FLAGS:=-m4a-nofpu -gdwarf-5

COMMON_FLAGS:=-flto -ffunction-sections -fdata-sections -gdwarf-5 -Og -m4a-nofpu
INCLUDES:=
WARNINGS:=-Wall -Wextra -Wno-main -pedantic -Werror -pedantic-errors

CC:=sh4aeb-elf-gcc
CC_FLAGS=-std=c11 $(COMMON_FLAGS) $(INCLUDES) $(WARNINGS)

CXX:=sh4aeb-elf-g++
CXX_FLAGS=-std=c++20 -fno-exceptions -Wno-write-strings $(COMMON_FLAGS) $(INCLUDES) $(WARNINGS)

LD:=sh4aeb-elf-g++
LD_FLAGS:=-nostartfiles -m4a-nofpu -Wl,--gc-sections -Wno-undef

READELF:=sh4aeb-elf-readelf
OBJCOPY:=sh4aeb-elf-objcopy

SOURCEDIR = src
BUILDDIR = obj
OUTDIR = dist

AS_SOURCES:=$(shell find $(SOURCEDIR) -name '*.S')
CC_SOURCES:=$(shell find $(SOURCEDIR) -name '*.c')
CXX_SOURCES:=$(shell find $(SOURCEDIR) -name '*.cpp')
OBJECTS := $(addprefix $(BUILDDIR)/,$(AS_SOURCES:.S=.o)) \
	$(addprefix $(BUILDDIR)/,$(CC_SOURCES:.c=.o)) \
	$(addprefix $(BUILDDIR)/,$(CXX_SOURCES:.cpp=.o))

CC_NOLTOSRCS:=$(shell find $(SOURCEDIR)/nolto -name '*.c')
CXX_NOLTOSRCS:=$(shell find $(SOURCEDIR)/nolto -name '*.cpp')
NOLTOOBJS := $(addprefix $(BUILDDIR)/,$(CC_NOLTOSRCS:.c=.o)) \
	$(addprefix $(BUILDDIR)/,$(CXX_NOLTOSRCS:.cpp=.o))

APP_ELF:=$(OUTDIR)/heapwalker.elf
APP_BIN:=$(OUTDIR)/heapwalker.bin

bin: $(APP_BIN) Makefile

hhk: $(APP_ELF) Makefile

all: $(APP_ELF) $(APP_BIN) Makefile

clean:
	rm -rf $(BUILDDIR) $(OUTDIR)

$(APP_BIN): $(APP_ELF)
	$(OBJCOPY) --output-target=binary --set-section-flags .bss=alloc,load,contents,data $(APP_ELF) $@

$(APP_ELF): $(OBJECTS) linker.ld
	mkdir -p $(dir $@)
	$(LD) -T linker.ld -Wl,-Map $@.map -o $@ $(LD_FLAGS) $(OBJECTS)

$(NOLTOOBJS): COMMON_FLAGS+=-fno-lto

$(BUILDDIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(AS) -c $< -o $@ $(AS_FLAGS)

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CC_FLAGS)

# Break the build if global constructors are present:
# Read the sections from the object file (with readelf -S) and look for any
# called .ctors - if they exist, give the user an error message, delete the
# object file (so that on subsequent runs of make the build will still fail)
# and exit with an error code to halt the build.
$(BUILDDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXX_FLAGS)
	@$(READELF) $@ -S | grep ".ctors" > /dev/null && echo "ERROR: Global constructors aren't supported." && rm $@ && exit 1 || exit 0

#clangd gets confused on these flags
compile_commands.json:
	$(MAKE) $(MAKEFLAGS) clean
	bear --output $@.tmp -- sh -c "$(MAKE) $(MAKEFLAGS) --keep-going all || exit 0"
	sed -i 's/-m4a-nofpu//' compile_commands.json.tmp
	mv compile_commands.json.tmp compile_commands.json

.PHONY: bin hhk all clean compile_commands.json