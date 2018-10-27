.PHONY: html

CFLAGS = -Wall -pedantic -g -O3 -Iparg

default: all

ORGLE=./orgle
WORGLE=./worgle
SORG=./sorg
EMACS=emacs

OBJ=worgle.o parg/parg.o

WORGLE_FLAGS=-Werror -g

all: $(ORGLE) $(WORGLE) $(SORG)

worgle.c: worgle.org
	$(ORGLE) $<

%.c: %.org
	$(WORGLE) $(WORGLE_FLAGS) $<

orgle: orgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

worgle: $(OBJ)
	$(CC) -std=c89 $(CFLAGS) $(OBJ) -o $@

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
	$(RM) orgle worgle sorg
	$(RM) worgle.c sorg.c
	$(RM) $(OBJ)
	$(RM) worgle.html worgle_toc.html
