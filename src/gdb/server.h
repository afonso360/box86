#ifndef __BOX86_GDB_SERVER_H_
#define __BOX86_GDB_SERVER_H_

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

#include "./packet.h"

static const size_t GDB_SERVER_QUEUE_SIZE = 16;

typedef struct gdb_server_s {
	uint16_t port;

	// Server socket
	int socketfd;

	// // Unprocessed packet recieve buffer, lock recvlock before reading / writing
	// pthread_mutex_t recvlock;
	// gdb_packet_t recvbuf[GDB_SERVER_QUEUE_SIZE];
	// size_t recv_read;
	// size_t recv_write;

	// // Packet send buffer, lock sendlock before reading / writing
	// pthread_mutex_t sendlock;
	// int sendnotiffd;
	// gdb_packet_t sendbuf[GDB_SERVER_QUEUE_SIZE];
	// size_t send_read;
	// size_t send_write;
} gdb_server_t;

int GDBServerStart(gdb_server_t *gdb);
int GDBServerStop(gdb_server_t *gdb);
int GDBServerWaitForConnection(gdb_server_t *gdb);

int GDBServerSendPacket(gdb_server_t *gdb, gdb_packet_t *packet);
int GDBServerRecievePacket(gdb_server_t *gdb, gdb_packet_t *packet);

int GDBServerNew(gdb_server_t *gdb);
void GDBServerFree(gdb_server_t *gdb);

#endif //__BOX86_GDB_SERVER_H_