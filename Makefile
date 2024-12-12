CFLAGS = -Wall -Werror -g -fsanitize=address
TARGETS = plaidsh plaidsh_test  # Updated to include plaidsh_test
OBJS = clist.o Tokenize.o pipeline.o parse.o ast.o plaidsh.o  # Added ast.o
HDRS = clist.h Token.h Tokenize.h pipeline.h ast.h  # Added ast.h
LIBS = -lasan -lm -lreadline

all: $(TARGETS)

# Linking the main executable
plaidsh: $(OBJS)
	gcc $(CFLAGS) $^ $(LIBS) -o $@

# Linking the test executable
plaidsh_test: plaidsh_test.o $(OBJS)  # Use plaidsh_test.o and existing object files
	gcc $(CFLAGS) $^ $(LIBS) -o $@

# Rule for plaidsh_test.o
plaidsh_test.o: plaidsh_test.c $(HDRS)
	gcc -c $(CFLAGS) $< -o $@

# Rules for object files with precise header dependencies
clist.o: clist.c clist.h
	gcc -c $(CFLAGS) $< -o $@

Tokenize.o: Tokenize.c Token.h Tokenize.h
	gcc -c $(CFLAGS) $< -o $@

pipeline.o: pipeline.c pipeline.h Token.h
	gcc -c $(CFLAGS) $< -o $@

parse.o: parse.c parse.h Token.h pipeline.h
	gcc -c $(CFLAGS) $< -o $@

ast.o: ast.c ast.h
	gcc -c $(CFLAGS) $< -o $@

plaidsh.o: plaidsh.c clist.h Token.h Tokenize.h pipeline.h ast.h
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS)
