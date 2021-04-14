CC=gcc
CFLAGS=-I.
DEPS=gameserver.h request.h game.h response.h
OBJ=server.o gameserver.o game.o response.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
