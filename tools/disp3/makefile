GCC= gcc
ifeq ($(shell uname -s), Darwin)
CFLAGS = -g
LFLAGS = -framework SDL2
else
CFLAGS = -g $(shell sdl2-config --cflags)
LFLAGS = $(shell sdl2-config --libs)
endif

EXE = dispsprite
OBS = $(addsuffix .o, $(basename $(wildcard *.c)))

%.o: %.c
	$(GCC) $(CFLAGS) -c $^

$(EXE): $(OBS)
	$(GCC) $(OBS) -o $(EXE) $(LFLAGS) 

clean:
	rm -f $(OBS)
	rm -f $(EXE)
