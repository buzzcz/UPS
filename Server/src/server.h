#ifndef SERVER
#define SERVER

struct sockaddr_in create_server_socket(int *server_socket);

void bind_server_socket(int server_socket, struct sockaddr_in server_addr, socklen_t server_addr_length);

void run_server(int server_socket);

#endif
