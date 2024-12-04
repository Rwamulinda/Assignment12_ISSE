CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=plaidsh
OBJS=clist.o expr_tree.o tokenize.o
HDRS=clist.h expr_tree.h token.h tokenize.h
LIBS=-lasan -lm -lreadline 


all: $(TARGETS)

plaidsh: $(OBJS) plaidsh.o
	gcc $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HDRS)
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS)
