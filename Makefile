.PHONY: html

CFLAGS = -Wall -std=c89 -pedantic -g -O3 -Iparg

default: all

# Orgle is a simple org tangler written C. It is used to build worglite.
ORGLE=./orgle

# Worglite is a liteweight version of worgle, used to build worgle components.
WORGLITE=./worglite
WORGLE=./worgle
SORG=./sorg
WORGMAP=./worgmap/worgmap
EMACS=emacs

OBJ=worgle.o db.o parg/parg.o

LIBS=-lsqlite3

WORGLE_FLAGS=-Werror -g

all: $(ORGLE) $(WORGLITE) $(WORGLE) $(SORG) $(WORGMAP)

worgle.c: worgle.org $(ORGLE)
	$(ORGLE) $<

%.c: %.org $(WORGLITE)
	$(WORGLITE) $(WORGLE_FLAGS) $<

# Worgle.c depends on db.o (and maybe other files)
worgle.o: db.o

$(ORGLE): orgle.c
	$(CC) $(CFLAGS) $< -o $@

$(WORGLE): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS)

$(WORGLITE): worgle.c parg/parg.o
	$(CC) -DWORGLITE $(CFLAGS) $^ -o $@

sorg: sorg.c parg/parg.o
	$(CC) -std=c89 $(CFLAGS) sorg.c parg/parg.o -o $@

install: all
	cp $(ORGLE) /usr/local/bin
	cp $(WORGLE) /usr/local/bin
	cp $(SORG) /usr/local/bin
	cp $(WORGMAP) /usr/local/bin

html: worgle.html

maps:
	$(WORGLE) $(WORGLE_FLAGS) -m worgle_map.org worgle.org
	$(SORG) -s worgle_map.org > worgle_map.html
	$(SORG) -t worgle_map.html -s worgle_map.org > worgle_map_toc.html
	$(WORGLE) $(WORGLE_FLAGS) -m sorg_map.org sorg.org
	$(SORG) -s sorg_map.org > sorg_map.html
	$(SORG) -t sorg_map.html -s sorg_map.org > sorg_map_toc.html

$(WORGMAP): $(WORGLE)
	cd worgmap; $(MAKE) -f Makefile

worgmap_clean:
	cd worgmap; $(MAKE) clean

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
	$(MAKE) worgmap_clean
