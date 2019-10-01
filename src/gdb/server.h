#ifndef __BOX86_GDB_SERVER_H_
#define __BOX86_GDB_SERVER_H_

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct gdb_server_s {
	uint16_t port;

	// Server socket
	int socketfd;
} gdb_server_t;

int GDBServerStart(gdb_server_t *gdb);
int GDBServerStop(gdb_server_t *gdb);
int GDBServerWaitForConnection(gdb_server_t *gdb);
int GDBServerNew(gdb_server_t *gdb);
void GDBServerFree(gdb_server_t *gdb);

#endif //__BOX86_GDB_SERVER_H_