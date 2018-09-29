CFLAGS = -Wall -pedantic -g -O3

default: all

ORGLE=./orgle

all: $(ORGLE) worgle

%.c: %.org
	$(ORGLE) $<

orgle: orgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

worgle: worgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

install: orgle
	cp orgle /usr/local/bin

clean:
	$(RM) orgle worgle
	$(RM) worgle.c
