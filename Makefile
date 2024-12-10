CFLAGS = -Wall -Werror -g -fsanitize=address
TARGETS = plaidsh
OBJS = clist.o Tokenize.o pipeline.o plaidsh.o
HDRS = clist.h Token.h Tokenize.h pipeline.h
LIBS = -lasan -lm -lreadline

all: $(TARGETS)

# Linking the final target
plaidsh: $(OBJS)
	gcc $(CFLAGS) $^ $(LIBS) -o $@

# Rules for object files with precise header dependencies
clist.o: clist.c clist.h
	gcc -c $(CFLAGS) $< -o $@

Tokenize.o: Tokenize.c Token.h Tokenize.h
	gcc -c $(CFLAGS) $< -o $@

pipeline.o: pipeline.c pipeline.h Token.h
	gcc -c $(CFLAGS) $< -o $@

plaidsh.o: plaidsh.c clist.h Token.h Tokenize.h pipeline.h
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS)
