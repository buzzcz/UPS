CC = gcc
BIN = server
OBJ = src/main.o src/server.o src/communication.o src/game.o

%.o:	%.c
	$(CC) -c $< -o $@

$(BIN): $(OBJ)
	${CC} $^ -o $@
	rm -f $(OBJ)

clean: 
	rm -f $(BIN)

