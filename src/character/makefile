IGNORE =

TOP_DIR := $(shell git rev-parse --show-toplevel)
include $(TOP_DIR)/makefile.inc

ifeq ($(UNIX),1)
	SUBDIRS = $(filter-out $(addsuffix /, $(IGNORE)), $(shell ls -p | grep /))
	OBS = $(filter-out $(IGNORE), $(addsuffix .o, $(basename $(wildcard *.c))))
else
	SUBDIRS = $(filter-out $(IGNORE), $(shell dir /B /AD))
	OBS = $(filter-out $(IGNORE), $(shell for /F %%i in ('dir /B /A-D *.c') do @echo %%~ni.o))
endif

$(SUBDIRS):
	$(MAKE) -C $@ all

.PHONY: $(SUBDIRS) all

all: $(OBS) $(SUBDIRS)
