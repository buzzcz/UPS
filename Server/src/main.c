#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"

int main(void) {
	int server_socket;
	socklen_t server_addr_length;
	struct sockaddr_in server_addr;

	server_addr = create_server_socket(&server_socket);
	server_addr_length = sizeof(server_addr);

	bind_server_socket(server_socket, server_addr, server_addr_length);

	run_server(server_socket);

	return 0;
}

