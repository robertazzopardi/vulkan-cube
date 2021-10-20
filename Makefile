#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
# CC = gcc
CC = clang

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -W -flto -ffast-math -Oz `sdl2-config --cflags`
CFLAGS  += -fsanitize=address -fno-omit-frame-pointer -ffunction-sections -fdata-sections
# CFLAGS  += --analyze
# CFLAGS  += -g

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS := -lvulkan -lSDL2 -lSDL2_image

# scan-build
SCAN =		scan-build

# scan flags
SCANFLAGS =	-v -analyze-headers -no-failure-reports -enable-checker deadcode.DeadStores --status-bugs

# define output directory
OUTPUT	:= bin

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

ifeq ($(OS),Windows_NT)
MAIN	:= main.exe
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= main
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files
OBJECTS		:= $(SOURCES:.c=.o)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: compile_shaders $(OUTPUT) $(MAIN)
# $(RM) $(OBJECTS)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)
# $(CC) $(CFLAGS) $(INCLUDES) -c $(SOURCES)
# $(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean_shaders:
	$(RM) $(SRC)/shaders/*.spv

.PHONY: clean
clean: clean_shaders
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) *.plist
	@echo Cleanup complete!

run: all
	$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!

check: clean all
	cppcheck -f --enable=all --inconclusive --check-library --debug-warnings --suppress=missingIncludeSystem --check-config $(INCLUDES) ./$(SRC)

scan_build: clean
	scan-build make
	$(RM) *.plist

compile_shaders: clean_shaders

# Acess vulkan sdk path and point to the glslc shader compiler
	GLSLC=$(VULKAN_SDK)/macOS/bin/glslc

# Compile the shaders to .spv files
	GLSLC $(SRC)/shaders/texture/shader.vert -o $(SRC)/shaders/vert.spv
	GLSLC $(SRC)/shaders/texture/shader.frag -o $(SRC)/shaders/frag.spv

# Dump the .spv files to a header file
	$(MD) $(INCLUDE)/shaders
	xxd -i $(SRC)/shaders/frag.spv > $(INCLUDE)/shaders/frag_shader.h
	xxd -i $(SRC)/shaders/vert.spv > $(INCLUDE)/shaders/vert_shader.h
