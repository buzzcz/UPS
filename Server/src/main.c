#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "server.h"

/*
 * Main entry point of the program
 * */
int main(int argc, char **argv) {
	int server_socket;
	socklen_t server_addr_length;
	struct sockaddr_in server_addr;

	if (argc != 3) {
		printf("Server must be started with 2 parameters: 1) INADDR_ANY / localhost / IP address, 2) port\n");
		exit(-1);
	}

	server_addr = create_server_socket(&server_socket, argv[1], atoi(argv[2]));
	server_addr_length = sizeof(server_addr);

	bind_server_socket(server_socket, server_addr, server_addr_length);

	run_server(server_socket);

	return 0;
}

