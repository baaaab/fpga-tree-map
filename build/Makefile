TARGET=mrpParser

BUILDDIR := $(shell pwd)
VPATH = $(shell find ../src -type d)
VPATH += $(BUILDDIR)
SRC_PATHS := ../src
SOURCES := $(shell find ../src -name "*.cpp")
OBJECTS := $(addprefix $(BUILDDIR)/,$(notdir $(SOURCES:%.cpp=%.o)))

CC=g++
LD=g++
CFLAGS=-MMD -std=c++11
CFLAGS+=-I../include
LDFLAGS= -lSDL -lX11
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

-include $(OBJECTS:.o=.d)

$(BUILDDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(dir $<) -c $< -o $@
	
clean:
	rm -f $(TARGET)
	rm -rf *.o
	rm -rf *.d

