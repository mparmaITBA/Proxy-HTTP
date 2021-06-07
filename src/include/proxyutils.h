#ifndef __PROXY_UTILS_H__
#define __PROXY_UTILS_H__

#define MAX_PENDING 10
#define MAX_CLIENTS 510
#define BUFFER_SIZE 65000
#define MAX_ADDR_BUFFER 128
// Constantes para acceder a los FdSets, BASE para el persistente, TMP para el que varia con select
#define BASE 0
#define TMP 1
#define FD_SET_ARRAY_SIZE 2

#include "connection.h"
#include <stddef.h>

typedef struct {
	char *host;
	char *service;
	pthread_t *main_thread_id;
	ConnectionNode *connection;
} ThreadArgs;

int setupPassiveSocket(const char *service);

void *resolve_addr(void *args);

int acceptConnection(int passiveSock);

typedef enum { WRITE, READ } OPERATION;
typedef enum { CLIENT, SERVER } PEER;

int handleConnection(ConnectionNode *node, ConnectionNode *prev, fd_set readFdSet[FD_SET_ARRAY_SIZE],
					 fd_set writeFdSet[FD_SET_ARRAY_SIZE], PEER peer);

ssize_t handle_operation(int fd, buffer *buffer, OPERATION operation, PEER peer, FILE *file);

int setup_connection(ConnectionNode *node, fd_set *writeFdSet);

int handle_client_connection(ConnectionNode *node, ConnectionNode *prev, fd_set read_fd_set[FD_SET_ARRAY_SIZE],
					 fd_set write_fd_set[FD_SET_ARRAY_SIZE]);

int handle_server_connection(ConnectionNode *node, ConnectionNode *prev, fd_set read_fd_set[FD_SET_ARRAY_SIZE],
					 fd_set write_fd_set[FD_SET_ARRAY_SIZE]);

#endif
