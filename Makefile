.PHONY: html

CFLAGS = -Wall -pedantic -g -O3 -Iparg

default: all

ORGLE=./orgle
WORGLE=./worgle
SORG=./sorg
EMACS=emacs

OBJ=worgle.o parg/parg.o
HTML=worgle.html

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

install: orgle worgle
	cp orgle /usr/local/bin
	cp worgle /usr/local/bin

html: $(HTML)

%.html: %.org
	$(EMACS) --batch -f package-initialize \
		-l org \
		--eval '(find-file "$<")' \
		--eval '(org-html-export-to-html)'

clean:
	$(RM) orgle worgle sorg
	$(RM) worgle.c
	$(RM) $(OBJ)
	$(RM) $(HTML)
