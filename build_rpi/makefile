TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)

SRC_PATHS+= ../src/core/
SRC_PATHS+= ../src/utils/
SRC_PATHS+= ../src/jpeg
SRC_PATHS+= ../src/rpi/
SRC_PATHS+= ../src/debug/
SRC_PATHS+= ../src/rest/
SRC_PATHS+= ../src/external_interface/

VPATH = $(shell find $(SRC_PATHS) -type d)
VPATH += $(BUILDDIR)

SOURCES := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CORE_PCH_FILENAME=/usr/include/opencv2/core/core.hpp
CORE_PCH=$(notdir $(CORE_PCH_FILENAME)).gch

CC=g++
LD=g++
#OPTIMISATION_FLAGS= -mfloat-abi=hard -mfpu=neon-vfpv4 -Ofast -ffast-math
#OPTIMISATION_FLAGS=-ggdb -O0 -fno-inline 
OPTIMISATION_FLAGS+= -Ofast
OPTIMISATION_FLAGS+= -marm
OPTIMISATION_FLAGS+= -mabi=aapcs-linux
OPTIMISATION_FLAGS+= -march=armv8-a+crc
OPTIMISATION_FLAGS+= -mfloat-abi=hard
OPTIMISATION_FLAGS+= -mfpu=crypto-neon-fp-armv8
OPTIMISATION_FLAGS+= -funsafe-math-optimizations
OPTIMISATION_FLAGS+= -mhard-float
OPTIMISATION_FLAGS+= -mlittle-endian
OPTIMISATION_FLAGS+= -mtune=cortex-a53
#OPTIMISATION_FLAGS+= -munaligned-access
OPTIMISATION_FLAGS+= -mvectorize-with-neon-quad
OPTIMISATION_FLAGS+= -ftree-vectorize
OPTIMISATION_FLAGS+= -ffast-math

CFLAGS=-MMD -std=c++11 -Wall $(OPTIMISATION_FLAGS) -pthread

CFLAGS+=-I. -I../include -pthread `pkg-config --cflags opencv`
CFLAGS+= -I/usr/local/include/raspicam
CFLAGS+= -I/opt/vc/include
CFLAGS+= -I/opt/vc/include/interface/
CFLAGS+= -I/opt/vc/include/interface/vcos/pthreads
CFLAGS+= -I/opt/vc/include/interface/vmcs_host/linux/

LDFLAGS+= $(OPTIMISATION_FLAGS) -lpthread -fwhole-program
LDFLAGS+= -lcurl -L/opt/vc/lib -lc -lm `pkg-config --libs opencv` -lmmal_core -lmmal_util -lmmal_vc_client -lvcos -lbcm_host -lboost_system -lboost_filesystem -lmicrohttpd

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d *.gch

