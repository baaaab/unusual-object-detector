TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)
VPATH = $(shell find ../src -type d)
VPATH += $(BUILDDIR)
SRC_PATHS := ../src
SOURCES := $(shell find ../src -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
CFLAGS=-O2 -MMD
CFLAGS+=-I../include
LDFLAGS=$(shell curl-config --libs)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf $(OBJECTS)
	rm -rf $(OBJECTS:.o=.d)


