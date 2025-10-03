// Header guard - prevents multiple inclusion
#ifndef IPC_API_H
#define IPC_API_H

// Function declarations (prototypes)
int ipc_socket_server_init(const int port);

int ipc_socket_client_init(const int port)

int ipc_socket_send(int socket_name, char *buffer_send, int buffer_size);

int ipc_socket_read(int socket_name, char *buffer_read, int buffer_size);

#endif