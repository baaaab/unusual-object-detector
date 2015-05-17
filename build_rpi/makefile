TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)
VPATH = $(shell find ../src -type d)
VPATH += $(BUILDDIR)

SRC_PATHS+= ../src/core
SRC_PATHS+= ../src/utils
SRC_PATHS+= ../src/jpeg/jpeg-compressor
SRC_PATHS+= ../src/rpi

SOURCES := $(shell find $(SRC_PATHS) -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
LD=g++
CFLAGS=-O2 -MMD -g -std=c++11 -Wall

CFLAGS+=-I../include -pthread `pkg-config --cflags opencv` -I/usr/local/include/raspicam

LDFLAGS=$(shell curl-config --libs) -lpthread -ljpeg -lc -lm `pkg-config --libs opencv` -lraspicam -lraspicam_cv
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d

