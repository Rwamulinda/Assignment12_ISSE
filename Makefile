CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=plaidsh
OBJS=clist.o Tokenize.o pipeline.o
HDRS=clist.h Token.h Tokenize.h pipeline.h
LIBS=-lasan -lm -lreadline

all: $(TARGETS)

plaidsh: $(OBJS) plaidsh.o
	gcc $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HDRS)
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS)
