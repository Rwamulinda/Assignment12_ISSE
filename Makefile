CFLAGS = -Wall -Werror -g -fsanitize=address
TARGETS = plaidsh plaidsh_test  # Updated to include plaidsh_test
OBJS = clist.o Tokenize.o pipeline.o parse.o ast.o # Added ast.o
HDRS = clist.h Token.h Tokenize.h pipeline.h ast.h  # Added ast.h
LIBS = -lasan -lm -lreadline

all: $(TARGETS)

# Linking the main executable
plaidsh: $(OBJS) plaidsh.o  
	gcc $(LDFLAGS) $^ $(LIBS) -o $@

# Linking the test executable
plaidsh_test: $(OBJS) plaidsh_test.o # Use plaidsh_test.o and existing object files
	gcc $(LDFLAGS) $^ $(LIBS) -o $@

# Rule for plaidsh_test.o
%.o: %.c $(HDRS)
	gcc -c $(CFLAGS) $< -o $@
clean:
	rm -f *.o $(TARGETS)
