.PHONY: html

CFLAGS = -Wall -std=c89 -pedantic -g -O3 -Iparg

default: all

# Orgle is a simple org tangler written C. It is used to build worglite.
ORGLE=./orgle

# Worglite is a liteweight version of worgle, used to build worgle components.
WORGLITE=./worglite
WORGLE=./worgle
SORG=./sorg
EMACS=emacs

OBJ=worgle.o db.o parg/parg.o

WORGLE_FLAGS=-Werror -g

all: $(ORGLE) $(WORGLITE) $(WORGLE) $(SORG)

worgle.c: worgle.org
	$(ORGLE) $<

%.c: %.org
	$(WORGLITE) $(WORGLE_FLAGS) $<

# Worgle.c depends on db.o (and maybe other files)
worgle.o: db.o

orgle: orgle.c
	$(CC) $(CFLAGS) $< -o $@

worgle: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

worglite: worgle.c parg/parg.o
	$(CC) -DWORGLITE $(CFLAGS) $^ -o $@

sorg: sorg.c parg/parg.o
	$(CC) -std=c89 $(CFLAGS) sorg.c parg/parg.o -o $@

install: all
	cp orgle /usr/local/bin
	cp worgle /usr/local/bin
	cp sorg /usr/local/bin

html: worgle.html

maps:
	$(WORGLE) $(WORGLE_FLAGS) -m worgle_map.org worgle.org
	$(SORG) -s worgle_map.org > worgle_map.html
	$(SORG) -t worgle_map.html -s worgle_map.org > worgle_map_toc.html
	$(WORGLE) $(WORGLE_FLAGS) -m sorg_map.org sorg.org
	$(SORG) -s sorg_map.org > sorg_map.html
	$(SORG) -t sorg_map.html -s sorg_map.org > sorg_map_toc.html

%.html: %.org
	$(SORG) -s $< > $@
	$(SORG) -t $@ -s $< > $*_toc.html

clean:
	$(RM) orgle worgle sorg worglite
	$(RM) $(OBJ)
	$(RM) worgle.html worgle_toc.html
	$(RM) sorg.c
	$(RM) worgle.c worgle.h
	$(RM) db.c db.h
