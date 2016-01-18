CC = gcc
BIN = server
OBJ = src/main.o src/server.o src/communication.o src/game.o src/list.o
CFLAGS = -lpthread

%.o:	%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(BIN): $(OBJ)
	${CC} $^ -o $@ $(CFLAGS)
	rm -f $(OBJ)

clean: 
	rm -f $(BIN)

