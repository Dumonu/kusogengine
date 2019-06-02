# Determine Paths
TOP_DIR := $(abspath .)
SRC_DIR := $(TOP_DIR)/src
OBJ_DIR := $(TOP_DIR)/build
INC_DIR := $(TOP_DIR)/include

# Set up basic flags
GCC := g++
EXE := dummy
CFLAGS := -std=c++11 -pedantic -I$(INC_DIR)
LFLAGS := 

# Modify flags and set up windows versus unix building
ifeq ($(OS),Windows_NT)
	CFLAGS += -DWINDOWS
	UNIX := 0
else ifeq ($(shell uname -s), Darwin)
	CFLAGS += -DMACOSX
	UNIX := 1
else
	CFLAGS += -DLINUX
	UNIX := 1
endif

# Find source files
ifeq ($(UNIX),1)
vpath %.cpp $(shell find $(SRC_DIR) -type d -printf "%p:")
vpath %.h $(shell find $(SRC_DIR) -type d -printf "%p:")

SRC := $(notdir $(shell find $(SRC_DIR) -name *.cpp))
INC := $(addprefix $(INC_DIR)/,$(notdir $(shell find $(SRC_DIR) -name *.h)))
else ifeq ($(UNIX),0)
vpath %.cpp $(SRC_DIR):$(shell for /F %%G in ('dir /S /B /A:D $(subst /,\,$(SRC_DIR))') do echo %%G:)
vpath %.h $(SRC_DIR):$(shell for /F %%G in ('dir /S /B /A:D $(subst /,\,$(SRC_DIR))') do echo %%G:)

SRC := $(notdir $(shell dir /S /B $(subst /,\,$(SRC_DIR))\*.cpp))
INC := $(notdir $(shell dir /S /B $(subst /,\,$(SRC_DIR))\*.h))
endif

OBJ := $(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(SRC)))
DOBJ := $(addprefix $(OBJ_DIR).dbg/,$(patsubst %.cpp,%.o,$(SRC)))

.PHONY: all release debug clean includes

all: debug release

release: $(OBJ_DIR) includes $(EXE)

debug: $(OBJ_DIR).dbg includes $(EXE).dbg

includes: $(INC_DIR) $(INC)

$(OBJ_DIR)/%.o: %.cpp $(INC)
	$(GCC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR).dbg/%.o: %.cpp $(INC)
	$(GCC) -c $(CFLAGS) -g -DDEBUG_ -o $@ $<

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ $(LFLAGS)

$(EXE).dbg: $(DOBJ)
	$(GCC) -o $@ $^ $(LFLAGS)

ifeq ($(UNIX),1)
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(INC_DIR):
	mkdir $(INC_DIR)

$(OBJ_DIR).dbg:
	mkdir $(OBJ_DIR).dbg

$(INC_DIR)/%.h: %.h
	cp -f $< $@

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OBJ_DIR).dbg
	rm -rf $(INC_DIR)
	rm -f $(EXE)
	rm -f $(EXE).dbg
else ifeq ($(UNIX),0)
$(OBJ_DIR):
	mkdir $(subst /,\,$(OBJ_DIR))

$(INC_DIR):
	mkdir $(subst /,\,$(INC_DIR))

$(OBJ_DIR).dbg:
	mkdir $(subst /,\,$(OBJ_DIR).dbg)

$(INC_DIR)/%.h: %.h
	copy /Y $< $@

clean:
	del $(subst /,\,$(OBJ_DIR))
	rd /Q $(subst /,\,$(OBJ_DIR))
	del $(subst /,\,$(OBJ_DIR).dbg)
	rd /Q $(subst /,\,$(OBJ_DIR).dbg)
	del $(subst /,\,$(INC_DIR))
	rd /Q $(subst /,\,$(INC_DIR))
	del $(subst /,\,$(EXE))
	del $(subst /,\,$(EXE).dbg)
endif