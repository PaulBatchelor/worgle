WORGLE ?=../worgle

CFLAGS=-O3 -Wall -pedantic -std=c89 -g

OBJ=worgmap.o get.o core.o dot.o html.o weewiki.o

LIBS=-lsqlite3

default: $(WORGLE) worgmap

worgmap: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS)

%.c: %.org
	$(WORGLE) -Werror -g $<

get.c: core.h

core.h: core.c

worgmap.c: core.h

install: worgmap
	cp worgmap /usr/local/bin

clean:
	$(RM) $(OBJ)
	$(RM) worgmap
	$(RM) *.c
	$(RM) *.h
