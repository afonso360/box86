#ifndef __BOX86_GDB_SERVER_H_
#define __BOX86_GDB_SERVER_H_

#include <stdint.h>
#include <stddef.h>

typedef struct gdb_server {
	uint16_t port;
	// Server socket
	int socketfd;
	// Connection socket
	int connsockfd;

	// Data recieved from socket
	char *recvbuf;
	size_t recvbuf_len;
	size_t recvbuf_alloc;
} gdb_server_t;

int GDBServerStart(gdb_server_t *gdb);
int GDBServerStop(gdb_server_t *gdb);
int GDBServerWaitForConnection(gdb_server_t *gdb);
int GDBServerNew(gdb_server_t *gdb);
void GDBServerFree(gdb_server_t *gdb);

// TODO: Do we want this public?
int __GDBServerReadSocket(gdb_server_t *gdb);

#endif //__BOX86_GDB_SERVER_H_