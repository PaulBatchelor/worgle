WORGLE=../worgle

OBJ=worgmap.o

default: $(WORGLE) worgmap

$(WORGLE):
	cd ..; make worgle

worgmap: $(OBJ)
	$(CC) $(CFLAGS) $< -o $@

%.c: %.org
	$(WORGLE) -Werror -g $<

clean:
	$(RM) $(OBJ)
	$(RM) worgmap
