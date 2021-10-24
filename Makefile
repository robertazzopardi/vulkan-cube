#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
# CC = gcc
CC = clang

# Acess vulkan sdk path and point to the glslc shader compiler
GLSLC=$(VULKAN_SDK)/macOS/bin/glslc

# define any compile-time flags
CFLAGS	:= -Wall -Wextra -W -flto -ffast-math -Oz `sdl2-config --cflags` -std=c18 -fpic -shared
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

# define examples directory
EXAMPLES := examples

ifeq ($(OS),Windows_NT)
MAIN	:= main.exe
LIB_NAME := libvk.so
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
else
MAIN	:= main
LIB_NAME := libvk.dylib
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

FRAG_SHADERS		:= $(wildcard $(patsubst %,%/*.frag, $(SOURCEDIRS)))
VERT_SHADERS		:= $(wildcard $(patsubst %,%/*.vert, $(SOURCEDIRS)))

# define the C object files
OBJECTS		:= $(SOURCES:.c=.o)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTLIB	:= $(call FIXPATH,$(OUTPUT)/$(LIB_NAME))
OUTPUTMAIN  := $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: compile_shaders $(OUTPUT) $(LIB_NAME)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(LIB_NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTLIB) $(OBJECTS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean_shaders:
	$(RM) $(wildcard $(patsubst %,%/*.spv, $(SOURCEDIRS)))

.PHONY: clean
clean: clean_shaders
	$(RM) $(OUTPUTLIB)
	$(RM) $(call FIXPATH,$(OBJECTS))
	$(RM) *.plist
	@echo Cleanup complete!

run: all
	$(OUTPUTLIB)
	@echo Executing 'run: all' complete!

run_sphere:
	$(CC) $(call FIXPATH,$(EXAMPLES)/main_sphere.c) -o $(OUTPUTMAIN) $(OUTPUTLIB) -I$(INCLUDE)
	$(OUTPUTMAIN)

check: clean all
	cppcheck -f --enable=all --inconclusive --check-library --debug-warnings --suppress=missingIncludeSystem --check-config $(INCLUDES) ./$(SRC)

scan_build: clean
	scan-build make
	$(RM) *.plist

compile_shaders: clean_shaders
	@echo ${VERT_SHADERS}

# for texture_path in $(VERT_SHADERS) ; do \
# 	echo $$texture_path ; \
# 	echo $$(dirname $$texture_path) ; \
# 	GLSLC $$texture_path -o $(call FIXPATH,$$(dirname $$texture_path)/vert.spv) ; \
# done
# echo include${$(dirname src/shaders/texture/shader.vert)#*src}

	for texture_type in light texture vertex ; do \
		GLSLC $(SRC)/shaders/$$texture_type/shader.vert -o $(SRC)/shaders/$$texture_type/vert.spv ; \
		GLSLC $(SRC)/shaders/$$texture_type/shader.frag -o $(SRC)/shaders/$$texture_type/frag.spv ; \
		$(MD) -p $(INCLUDE)/shaders/$$texture_type ; \
		xxd -i -C $(SRC)/shaders/$$texture_type/frag.spv > $(INCLUDE)/shaders/$$texture_type/$${texture_type}_frag_shader.h ; \
		echo "#ifndef $${texture_type}_FRAG_SHADER\n#define $${texture_type}_FRAG_SHADER\n" | \
			cat - $(INCLUDE)/shaders/$$texture_type/$${texture_type}_frag_shader.h > temp && \
			mv temp $(INCLUDE)/shaders/$$texture_type/$${texture_type}_frag_shader.h && echo "\n#endif" >> \
			$(INCLUDE)/shaders/$$texture_type/$${texture_type}_frag_shader.h ; \
		xxd -i -C $(SRC)/shaders/$$texture_type/vert.spv > $(INCLUDE)/shaders/$$texture_type/$${texture_type}_vert_shader.h ; \
		echo "#ifndef $${texture_type}_VERT_SHADER\n#define $${texture_type}_VERT_SHADER\n" | \
			cat - $(INCLUDE)/shaders/$$texture_type/$${texture_type}_vert_shader.h > \
			temp && mv temp $(INCLUDE)/shaders/$$texture_type/$${texture_type}_vert_shader.h && \
			echo "\n#endif" >> $(INCLUDE)/shaders/$$texture_type/$${texture_type}_vert_shader.h ; \
	done
