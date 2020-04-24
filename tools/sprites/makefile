GCC= gcc
CFLAGS= -g
LFLAGS =

EXE = ff2animage
OBS = $(addsuffix .o, $(basename $(wildcard *.c)))

%.o: %.c
	$(GCC) $(CFLAGS) -c $^

$(EXE): $(OBS)
	$(GCC) $(OBS) -o $(EXE) $(LFLAGS) 

clean:
	rm -f $(OBS)
	rm -f $(EXE)
