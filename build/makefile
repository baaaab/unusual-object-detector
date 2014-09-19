TARGET=unusual_object_detector

BUILDDIR := $(shell pwd)
VPATH = $(shell find ../src -type d)
VPATH += $(BUILDDIR)
SRC_PATHS := ../src
SOURCES := $(shell find ../src -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
LD=g++
CFLAGS=-O2 -MMD
CFLAGS+=-I../include -pthread
LDFLAGS=$(shell curl-config --libs) -lpthread -lSDL -ljpeg -lc -lm -lX11
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf $(OBJECTS)
	rm -rf $(OBJECTS:.o=.d)


