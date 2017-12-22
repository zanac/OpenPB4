DEBUG=0
WANT_LIBCO = 1
DISABLE_ERROR_LOGGING = 1
#set later according to platform
IS_X86 = 0
TARGET_NAME = mame2000

CORE_DIR = .

ifeq (,$(findstring msvc,$(platform)))
GCC_DEFINES := -Wno-sign-compare -Wunused -Wpointer-arith -Waggregate-return -Wshadow
endif

GIT_VERSION ?= " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
	CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
	EXE_EXT = .exe
	system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	system_platform = osx
ifeq ($(shell uname -p),powerpc)
	arch = ppc
else
	arch = intel
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
	system_platform = win
endif

TARGET := $(TARGET_NAME)_libretro.so
fpic := -fPIC
SHARED := -shared -Wl,--version-script=link.T -Wl,-no-undefined -lpthread
SHARED := -shared -Wl,--version-script=link.T -Wl,-no-undefined -lpthread

CFLAGS += -fsigned-char -fPIC -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fomit-frame-pointer -ffast-math
USE_CYCLONE = 1
#USE_DRZ80 = 1

ifeq ($(DEBUG), 1)
CFLAGS += -O0 -g -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fomit-frame-pointer -ffast-math -fPIC
else
CFLAGS += -O2 -DNDEBUG -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fomit-frame-pointer -ffast-math -fPIC
endif

# compiler, linker and utilities
RM = rm -f
LIBS = -lm
OBJ = obj

ifeq ($(HAVE_RZLIB),)
ZLIB_INCLUDE := -Isrc/zlib
endif

INCFLAGS :=

# include the various .mak files
include Makefile.common

DEFS += $(fpic) $(PLATFORM_DEFINES) $(ZLIB_INCLUDE) $(GCC_DEFINES) $(INCFLAGS) $(INCFLAGS_PLATFORM)
# combine the various definitions to one
CDEFS +=  $(ENDIANNESS_DEFINES) $(DEFS) $(COREDEFS) $(CPUDEFS) $(SOUNDDEFS)

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_ASM:.s=.o)

OBJOUT   = -o
LINKOUT  = -o 

ifneq (,$(findstring msvc,$(platform)))
	OBJOUT = -Fo
	LINKOUT = -out:
	LD = link.exe
else
	LD = $(CC)
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	@echo Linking $@...
	$(LD) $(SHARED) $(LDFLAGS) $(OBJECTS) $(LIBS) $(LINKOUT)$@
endif
%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< $(OBJOUT)$@

%.o: %.s
	$(CC) $(CDEFS) $(CFLAGS) -c $< $(OBJOUT)$@

%.o: %.S
	$(CC) $(CDEFS) $(CFLAGS) -c $< $(OBJOUT)$@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
