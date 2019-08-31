WORGLE=../worgle

OBJ=worgmap.o get.o core.o

default: $(WORGLE) worgmap

$(WORGLE):
	cd ..; make worgle

worgmap: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

%.c: %.org
	$(WORGLE) -Werror -g $<

clean:
	$(RM) $(OBJ)
	$(RM) worgmap
