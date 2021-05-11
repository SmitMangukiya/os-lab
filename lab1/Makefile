# lab5
# IT215 Winter 2018
                                 
TARGETS = mmame minishell
all: $(TARGETS)
                                 
clean: 
	rm -f *.o *~ $(TARGETS)


minishell: minishell.c
	gcc -o minishell minishell.c

mmame: doublelist.o doublesort.o
	gcc -o mmame doublelist.o doublesort.o mmame.c

doublelist.o: doublelist.c doublelist.h
	gcc -c doublelist.c

doublesort.o: doublesort.c doublesort.h
	gcc -c doublesort.c

