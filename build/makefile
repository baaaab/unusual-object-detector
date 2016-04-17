TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)

SRC_PATHS+= ../src/core/
SRC_PATHS+= ../src/utils/
SRC_PATHS+= ../src/jpeg/turbojpeg/
SRC_PATHS+= ../src/x86/

VPATH = $(shell find $(SRC_PATHS) -type d)
VPATH += $(BUILDDIR)

SOURCES := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CORE_PCH_FILENAME=/usr/include/opencv2/core/core.hpp
CORE_PCH=$(notdir $(CORE_PCH_FILENAME)).gch

CC=g++
LD=g++
CFLAGS=-O0 -MMD -g -std=c++11
CFLAGS+= -I../include -pthread
CFLAGS+= -Wall -Werror

PCHFLAGS := $(addprefix , $(CFLAGS))

LDFLAGS+= -fwhole-program  $(OPTIMISATION_FLAGS) -pthread
LDFLAGS+= $(shell curl-config --libs) -lpthread -lSDL -ljpeg -lc -lm -lX11 -lturbojpeg -lboost_system
LDFLAGS+= -lopencv_core -lopencv_imgproc -lopencv_highgui

all: $(CORE_PCH) $(TARGET)

$(CORE_PCH):
	$(CC) $(PCHFLAGS) -x c++-header $(CORE_PCH_FILENAME) -o $(CORE_PCH)

$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d *.gch

