CC=gcc
CFLAGS= -g -Wall -Wpedantic -std=gnu17
OBJDIR=./obj
INCDIR=./inc
SRCDIR=./src

SRCS:=$(wildcard ${SRCDIR}/*.c)
OBJS:=$(patsubst ${SRCDIR}/%.c,${OBJDIR}/%.o,${SRCS})

all: $(OBJS)
	gcc -o a.out $(CFLAGS) $(OBJS)

run: all 
	./a.out ArrayInit.xme

debug: all
	./a.out --debug ArrayInit.xme

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm $(OBJDIR)/*.o *.out

.PHONY: all clean run debug
