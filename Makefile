CFLAGS = -Wall -pedantic -g -O3

orgle: orgle.c
	$(CC) -std=c89 $(CFLAGS) $< -o $@

install: orgle
	cp orgle /usr/local/bin

clean:
	$(RM) orgle
