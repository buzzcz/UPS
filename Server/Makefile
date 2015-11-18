CC = gcc
BIN = server
OBJ = server.o

%.o:	clean %.c
	$(CC) -c $< -o $@

$(BIN): $(OBJ)
	${CC} $^ -o $@
	rm -f $(OBJ)

clean: 
	rm -f $(BIN)

