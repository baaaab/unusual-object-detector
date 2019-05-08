TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)

SRC_PATHS+= ../src/core/
SRC_PATHS+= ../src/utils/
SRC_PATHS+= ../src/jpeg
SRC_PATHS+= ../src/x86/
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
CFLAGS=-Ofast -MMD -std=c++11 -march=native
CFLAGS+= -I../include -pthread
CFLAGS+= -Wall -Werror -ggdb

PCHFLAGS := $(addprefix , $(CFLAGS))

LDFLAGS+= -fwhole-program  $(OPTIMISATION_FLAGS) -pthread
LDFLAGS+= $(shell curl-config --libs) -lpthread -ljpeg -lc -lm -lboost_system -lboost_filesystem -lmicrohttpd
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

