# Determine Paths
TOP_DIR := $(abspath .)
SRC_DIR := $(TOP_DIR)/src
DEP_DIR := $(TOP_DIR)/dep
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

include $(addprefix $(DEP_DIR)/,$(patsubst %.cpp,%.d,$(SRC)))

$(OBJ_DIR)/%.o: %.cpp
	$(GCC) -c $(CFLAGS) -o $@ $<

$(OBJ_DIR).dbg/%.o: %.cpp
	$(GCC) -c $(CFLAGS) -g -DDEBUG_ -o $@ $<

$(EXE): $(OBJ)
	$(GCC) -o $@ $^ $(LFLAGS)

$(EXE).dbg: $(DOBJ)
	$(GCC) -o $@ $^ $(LFLAGS)

ifeq ($(UNIX),1)
$(DEP_DIR)/%.d: %.cpp $(DEP_DIR)
	$(GCC) -MM $(CFLAGS) $< | awk 'BEGIN {FS="[:]";OFS=":";RS=""}{$$1="$(OBJ_DIR)/" $$1 " $(OBJ_DIR).dbg/" $$1 " $@"; print $$0}' > $@

$(DEP_DIR):
	mkdir $(DEP_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(INC_DIR):
	mkdir $(INC_DIR)

$(OBJ_DIR).dbg:
	mkdir $(OBJ_DIR).dbg

$(INC_DIR)/%.h: %.h
	cp -f $< $@

clean:
	rm -rf $(DEP_DIR)
	rm -rf $(OBJ_DIR)
	rm -rf $(OBJ_DIR).dbg
	rm -rf $(INC_DIR)
	rm -f $(EXE)
	rm -f $(EXE).dbg
else ifeq ($(UNIX),0)
$(DEP_DIR)/%.d: %.cpp $(DEP_DIR)
	$(GCC) -MM $(CFLAGS) $< | awk "BEGIN {FS=\"[:]\";OFS=\":\";RS=\"\"}{$$1=\"$(OBJ_DIR)/\" $$1 \" $(OBJ_DIR).dbg/\" $$1 \" $@\"; print $$0}" > $@

$(DEP_DIR):
	mkdir $(subst /,\,$(DEP_DIR))

$(OBJ_DIR):
	mkdir $(subst /,\,$(OBJ_DIR))

$(INC_DIR):
	mkdir $(subst /,\,$(INC_DIR))

$(OBJ_DIR).dbg:
	mkdir $(subst /,\,$(OBJ_DIR).dbg)

$(INC_DIR)/%.h: %.h
	copy /Y $< $@

clean:
	rd  /S /Q $(subst /,\,$(DEP_DIR)) || Echo:
	rd  /S /Q $(subst /,\,$(OBJ_DIR)) || Echo:
	rd  /S /Q $(subst /,\,$(OBJ_DIR).dbg) || Echo:
	rd  /S /Q $(subst /,\,$(INC_DIR)) || Echo:
	del /F /Q $(subst /,\,$(EXE).exe)
	del /F /Q $(subst /,\,$(EXE).dbg)
endif