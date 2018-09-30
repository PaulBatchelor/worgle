CFLAGS = -Wall -pedantic -g -O3 -Iparg

default: all

ORGLE=./orgle

OBJ=worgle.o parg/parg.o

all: $(ORGLE) worgle

%.c: %.org
	$(ORGLE) $<

orgle: orgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

worgle: $(OBJ)
	$(CC) -std=c89 $(CFLAGS) $(OBJ) -o $@

install: orgle
	cp orgle /usr/local/bin

clean:
	$(RM) orgle worgle
	$(RM) worgle.c
	$(RM) $(OBJ)
