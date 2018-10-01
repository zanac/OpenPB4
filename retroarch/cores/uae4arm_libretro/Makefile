
PLATFLAGS :=

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
else ifneq ($(findstring arm,$(shell uname -a)),)
    PLATFLAGS +=  -DARM  -marm	
endif
endif

PLATFLAGS +=  -DARM  -marm	

TARGET_NAME := uae4arm

CORE_DIR  := .
ROOT_DIR  := .

TARGET := $(TARGET_NAME)_libretro.so
fpic = -fPIC
SHARED :=-shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,--no-undefined
PLATFORM_DEFINES += -marm -mcpu=cortex-a9 -mfpu=neon-vfpv4 -mfloat-abi=hard -ftree-vectorize
CPU_FLAGS +=  -marm -mcpu=cortex-a9 -mfloat-abi=hard -D__arm__ -DARM_ASM -D__NEON_OPT
PLATFORM_DEFINES +=   -DLSB_FIRST -DALIGN_DWORD -DWITH_LOGGING -mfpu=neon-vfpv4 -mfloat-abi=hard
HAVE_NEON = 1
USE_PICASSO96 = 1
CFLAGS += $(PLATFORM_DEFINES)
CXXFLAGS += $(PLATFORM_DEFINES)
CC = gcc
CXX = g++ 
LDFLAGS += -lz -lpthread

CFLAGS += -O3 -fomit-frame-pointer -finline -fno-builtin -ffast-math

DEFINES += 

DEFS += -DCPU_arm -DARM_ASSEMBLY -DARMV6_ASSEMBLY -DGP2X -DPANDORA -DSIX_AXIS_WORKAROUND
DEFS += -DROM_PATH_PREFIX=\"./\" -DDATA_PREFIX=\"./data/\" -DSAVE_PREFIX=\"./saves/\"
DEFS += -DRASPBERRY
#-DANDROIDSDL 

DEFS += -DPICASSO96

DEFS += -DUSE_ARMNEON

DEFINES += -D__LIBRETRO__ $(DEFS)
#-std=gnu99 
CFLAGS += $(DEFINES) -DRETRO=1 -DINLINE="inline" $(CPU_FLAGS) -fexceptions -fpermissive

include Makefile.common

OBJECTS += $(SOURCES_C:.c=.o) $(SOURCES_CXX:.cpp=.o) $(SOURCES_ASM:.S=.o)

INCDIRS := $(EXTRA_INCLUDES) $(INCFLAGS)


OBJECTS += $(OBJS)

all: $(TARGET)

ifeq ($(platform), wii)
$(TARGET): $(OBJECTS) 
	$(AR) rcs $@ $(OBJECTS) 
else ifeq ($(platform), ps3)
$(TARGET): $(OBJECTS) 
	$(AR) rcs $@ $(OBJECTS) 

else ifeq ($(platform), win)
$(TARGET): $(OBJECTS)
	$(CC) $(fpic) $(SHARED) $(INCDIRS) -o $@ $(OBJECTS) $(LDFLAGS)
else
$(TARGET): $(OBJECTS)
	$(CXX) $(fpic) $(SHARED) $(INCDIRS) -o $@ $(OBJECTS) $(LDFLAGS)

endif

$(EMU)/od-retro/neon_helper.o: $(EMU)/od-retro/neon_helper.s
	$(CXX) $(CPU_FLAGS) $(PLATFORM_DEFINES) -Wall -o $(EMU)/od-retro/neon_helper.o -c $(EMU)/od-retro/neon_helper.s
	echo $(OBJS)

%.o: %.cpp
	$(CXX) $(fpic) $(CFLAGS) $(PLATFLAGS) $(INCDIRS) -c -o $@ $<

%.o: %.c
	$(CC) $(fpic) $(CFLAGS) $(PLATFLAGS) $(INCDIRS) -c -o $@ $<

%.o: %.S
	$(CC_AS) $(CFLAGS)  $(PLATFLAGS) -c $^ -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) 

.PHONY: clean

