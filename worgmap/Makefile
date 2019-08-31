WORGLE=../worgle

CFLAGS=-O3 -Wall -pedantic -std=c89

OBJ=worgmap.o get.o core.o

LIBS=-lsqlite3

default: $(WORGLE) worgmap

$(WORGLE):
	cd ..; make worgle

worgmap: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS)

%.c: %.org
	$(WORGLE) -Werror -g $<

clean:
	$(RM) $(OBJ)
	$(RM) worgmap
