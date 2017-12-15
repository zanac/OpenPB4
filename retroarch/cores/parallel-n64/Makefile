DEBUG=0
PERF_TEST=0
HAVE_SHARED_CONTEXT=0
WITH_CRC=brumme
FORCE_GLES=1
HAVE_OPENGL=1
HAVE_VULKAN_DEBUG=0
GLIDEN64=0
GLIDEN64CORE=0
GLIDEN64ES=0
HAVE_RSP_DUMP=0
HAVE_RDP_DUMP=0
HAVE_RICE=1
HAVE_PARALLEL=1
HAVE_PARALLEL_RSP=0
STATIC_LINKING=0

DYNAFLAGS :=
INCFLAGS  :=
COREFLAGS :=
CPUFLAGS  :=
GLFLAGS   :=

platform="odroid"

UNAME=$(shell uname -a)

SPACE :=
SPACE := $(SPACE) $(SPACE)
BACKSLASH :=
BACKSLASH := \$(BACKSLASH)
filter_out1 = $(filter-out $(firstword $1),$1)
filter_out2 = $(call filter_out1,$(call filter_out1,$1))

ifeq ($(HAVE_VULKAN_DEBUG),1)
HAVE_RSP_DUMP=1
HAVE_RDP_DUMP=1
endif

# Dirs
ROOT_DIR := .
LIBRETRO_DIR := $(ROOT_DIR)/libretro

ifeq ($(platform),)
   platform = unix
   ifeq ($(UNAME),)
      platform = win
   else ifneq ($(findstring MINGW,$(UNAME)),)
      platform = win
   else ifneq ($(findstring Darwin,$(UNAME)),)
      platform = osx
   else ifneq ($(findstring win,$(UNAME)),)
      platform = win
   endif
else ifneq (,$(findstring armv,$(platform)))
   override platform += unix
else ifneq (,$(findstring rpi,$(platform)))
   override platform += unix
else ifneq (,$(findstring odroid,$(platform)))
   override platform += unix
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
   EXE_EXT = .exe
   system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   system_platform = osx
   arch = intel
ifeq ($(shell uname -p),powerpc)
   arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   system_platform = win
endif

# Cross compile ?

ifeq (,$(ARCH))
   ARCH = $(shell uname -m)
endif

# Target Dynarec
WITH_DYNAREC = $(ARCH)

ifeq ($(ARCH), $(filter $(ARCH), i386 i686))
   WITH_DYNAREC = x86
else ifeq ($(ARCH), $(filter $(ARCH), arm))
   WITH_DYNAREC = arm
endif

ifeq ($(HAVE_VULKAN_DEBUG),1)
TARGET_NAME := parallel_n64_debug
else
TARGET_NAME := parallel_n64
endif

CC_AS ?= $(CC)

GIT_VERSION ?= " $(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION)," unknown")
	COREFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

# Unix
TARGET := $(TARGET_NAME)_libretro.so
LDFLAGS += -shared -Wl,--no-undefined
     ifeq ($(DEBUG_JIT),)
        LDFLAGS += -Wl,--version-script=$(LIBRETRO_DIR)/link.T
     endif
fpic = -fPIC

HAVE_THR_AL=1

#ifeq ($(WITH_DYNAREC), $(filter $(WITH_DYNAREC), x86_64 x64))
#ifeq ($(HAVE_PARALLEL), 1)
	#HAVE_PARALLEL_RSP=1
#endif
#endif

ifeq ($(FORCE_GLES),1)
   GLES = 1
   GL_LIB := -lGLESv2
else ifneq (,$(findstring gles,$(platform)))
   GLES = 1
   GL_LIB := -lGLESv2
else
   GL_LIB := -lGL
endif
# ODROIDs

BOARD := $(shell cat /proc/cpuinfo | grep -i odroid | awk '{print $$3}')
GLES = 1
GL_LIB := -lGLESv2
CPUFLAGS += -DNO_ASM -DARM -D__arm__ -DARM_ASM -D__NEON_OPT -DNOSSE
CPUFLAGS += -marm -mfloat-abi=hard
CPUFLAGS += -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -ffast-math
HAVE_NEON = 1
WITH_DYNAREC=arm

# Generic ARM
CPUFLAGS += -DNO_ASM -DARM -D__arm__ -DARM_ASM -DNOSSE
WITH_DYNAREC=arm
CPUFLAGS += -D__NEON_OPT -mfpu=neon
HAVE_NEON = 1

PLATFORM_EXT := unix

ifneq ($(SANITIZER),)
    CFLAGS   += -fsanitize=$(SANITIZER)
    CXXFLAGS += -fsanitize=$(SANITIZER)
    LDFLAGS  += -fsanitize=$(SANITIZER)
endif

ifeq ($(HAVE_OPENGL),0)
	HAVE_GLIDE64=0
	HAVE_GLN64=0
	HAVE_GLIDEN64=0
	HAVE_RICE=0
endif

include Makefile.common

ifeq ($(HAVE_NEON), 1)
   COREFLAGS += -DHAVE_NEON
endif

ifeq ($(PERF_TEST), 1)
   COREFLAGS += -DPERF_TEST
endif

ifeq ($(HAVE_SHARED_CONTEXT), 1)
   COREFLAGS += -DHAVE_SHARED_CONTEXT
endif

COREFLAGS += -D__LIBRETRO__ -DM64P_PLUGIN_API -DM64P_CORE_PROTOTYPES -D_ENDUSER_RELEASE -DSINC_LOWER_QUALITY

OBJOUT   = -o
LINKOUT  = -o 

ifneq (,$(findstring msvc,$(platform)))
	OBJOUT = -Fo
	LINKOUT = -out:
	LD = link.exe
else
	LD = $(CXX)
endif

ifeq ($(DEBUG), 1)
   CPUOPTS += -DOPENGL_DEBUG

   ifneq (,$(findstring msvc,$(platform)))
      ifeq (,$(findstring msvc2005,$(platform)))
      ifeq (,$(findstring msvc2010,$(platform)))
				 # /FS was added in 2013
         CPUOPTS += -FS
      endif
      endif
      CPUOPTS += -Od -MDd -Zi
      LDFLAGS += -DEBUG
   else
      CPUOPTS += -O0 -g
   endif
else
   CPUOPTS += -O2 -DNDEBUG

endif

WANT_CXX11=0

ifeq ($(platform), qnx)
   CFLAGS   += -Wp,-MMD
   CXXFLAGS += -Wp,-MMD
else
ifeq ($(GLIDEN64),1)
   CFLAGS   += -DGLIDEN64
   CXXFLAGS += -DGLIDEN64
   WANT_CXX11=1
   CFLAGS   += -MMD
   CXXFLAGS += -MMD
else ifeq ($(HAVE_PARALLEL), 1)
   WANT_CXX11=1
   ifeq (,$(findstring msvc,$(platform)))
     CFLAGS   += -MMD
     CXXFLAGS += -MMD
   endif
else ifeq (,$(findstring msvc,$(platform)))
    CFLAGS   += -MMD
    CXXFLAGS += -std=c++98 -MMD
endif
ifeq ($(GLIDEN64ES),1)
   CFLAGS   += -DGLIDEN64ES
   CXXFLAGS += -DGLIDEN64ES
endif
ifeq ($(GLIDEN64CORE),1)
   CFLAGS += -DCORE
   CXXFLAGS += -DCORE
endif
endif

ifeq ($(HAVE_THR_AL), 1)
WANT_CXX11=1
endif

ifeq ($(WANT_CXX11),1)
ifeq (,$(findstring msvc,$(platform)))
CXXFLAGS += -std=c++0x 
endif
endif

ifneq (,$(findstring msvc,$(platform)))
CFLAGS += -DINLINE="_inline"
CXXFLAGS += -DINLINE="_inline"
else
CFLAGS += -DINLINE="inline"
CXXFLAGS += -DINLINE="inline"
endif

ASFLAGS     := $(ASFLAGS) $(CFLAGS)

### Finalize ###
OBJECTS     += $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o) $(SOURCES_ASM:.S=.o)
CXXFLAGS    += $(CPUOPTS) $(COREFLAGS) $(INCFLAGS) $(INCFLAGS_PLATFORM) $(PLATCFLAGS) $(fpic) $(PLATCFLAGS) $(CPUFLAGS) $(GLFLAGS) $(DYNAFLAGS)
CFLAGS      += $(CPUOPTS) $(COREFLAGS) $(INCFLAGS) $(INCFLAGS_PLATFORM) $(PLATCFLAGS) $(fpic) $(PLATCFLAGS) $(CPUFLAGS) $(GLFLAGS) $(DYNAFLAGS)

ifeq ($(findstring Haiku,$(UNAME)),)
ifeq (,$(findstring msvc,$(platform)))
   LDFLAGS += -lm
endif
endif

LDFLAGS    += $(fpic)

ifeq ($(platform), theos_ios)
COMMON_FLAGS := -DIOS $(COMMON_DEFINES) $(INCFLAGS) $(INCFLAGS_PLATFORM) -I$(THEOS_INCLUDE_PATH) -Wno-error
$(LIBRARY_NAME)_ASFLAGS += $(CFLAGS) $(COMMON_FLAGS)
$(LIBRARY_NAME)_CFLAGS += $(CFLAGS) $(COMMON_FLAGS)
$(LIBRARY_NAME)_CXXFLAGS += $(CXXFLAGS) $(COMMON_FLAGS)
${LIBRARY_NAME}_FILES = $(SOURCES_CXX) $(SOURCES_C) $(SOURCES_ASM)
${LIBRARY_NAME}_FRAMEWORKS = OpenGLES
${LIBRARY_NAME}_LIBRARIES = z
include $(THEOS_MAKE_PATH)/library.mk
else
all: $(TARGET)
$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(LD) $(LINKOUT)$@ $(OBJECTS) $(LDFLAGS) $(GL_LIB) $(LIBS)
endif



%.o: %.S
ifneq (,$(findstring msvc,$(platform)))
	$(CC_AS) $(ASFLAGS) -o$@ $< 
else
	$(CC_AS) $(ASFLAGS) -c $< $(OBJOUT)$@
endif

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< $(OBJOUT)$@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< $(OBJOUT)$@


clean:
	rm -f $(OBJECTS) $(TARGET) $(OBJECTS:.o=.d)

.PHONY: clean
-include $(OBJECTS:.o=.d)
endif

print-%:
	@echo '$*=$($*)'
