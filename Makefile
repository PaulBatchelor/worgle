.PHONY: html

CFLAGS = -Wall -pedantic -g -O3 -Iparg

default: all

ORGLE=./orgle
EMACS=emacs

OBJ=worgle.o parg/parg.o
HTML=worgle.html

all: $(ORGLE) worgle

%.c: %.org
	$(ORGLE) $<

orgle: orgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

worgle: $(OBJ)
	$(CC) -std=c89 $(CFLAGS) $(OBJ) -o $@

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
	$(RM) orgle worgle
	$(RM) worgle.c
	$(RM) $(OBJ)
	$(RM) $(HTML)
