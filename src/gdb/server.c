#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include "../include/debug.h"
#include "./server.h"
#include "./packet.h"

static const size_t RECVBUF_START_SIZE = 64;

typedef struct server_connection_data_s {
	int fd;

	pthread_t thread;

	// Data recieved from socket
	char *recvbuf;
	size_t recvbuf_len;
	size_t recvbuf_alloc;

	// TODO: Should we just share the packet queue?
	gdb_server_t *gdb;
} server_connection_data_t;


// Returns the number of bytes read, or a negative number when an error occurs
int GDBServerReadSocket(server_connection_data_t *conn) {
	assert(conn != NULL && "NULL gdb struct");


	char *buf = &conn->recvbuf[conn->recvbuf_len];
	size_t buf_len = conn->recvbuf_alloc - conn->recvbuf_len;
	ssize_t read_len = read(conn->fd, buf, buf_len);
	if (read_len < 0) {
		return read_len;
	}

	conn->recvbuf_len = conn->recvbuf_len + read_len;

	// If we read all of the available size, try again after resizing,
	// because there might be more data.
	if ((size_t) read_len == buf_len) {
		size_t newalloc = conn->recvbuf_alloc * 2;
		printf_log(LOG_DEBUG, "conn: Resizing read buffer to: %zu\n", newalloc);
		char *newbuf = realloc(conn->recvbuf, newalloc);
		if (newbuf == NULL) {
			return -ENOMEM;
		}


		conn->recvbuf = newbuf;
		conn->recvbuf_alloc = newalloc;

		// Retry to read with the new buffer size;
		int rret = GDBServerReadSocket(conn);
		return (rret < 0) ? rret : (rret + read_len);
	}

	return read_len;
}


void *GDBServerThreadLoop(void *v_conn) {
	assert(v_conn != NULL && "NULL gdb struct");
	server_connection_data_t *conn = (server_connection_data_t *) v_conn;
	printf_log(LOG_DEBUG, "GDB: Started listening thread: %d\n", conn->fd);

	
	
	int err = 0;
	
	conn->recvbuf_len = 0;
	conn->recvbuf_alloc = RECVBUF_START_SIZE;
	conn->recvbuf = calloc(conn->recvbuf_alloc, sizeof(char));
	if (conn->recvbuf == NULL)  {
		printf_log(LOG_INFO, "GDB: Failed to allocate recieve buffer\n");
		goto exit;
	}


	while (true) {
		
		printf("GDB: Reading Socket");
		err = GDBServerReadSocket(conn);
		if (err < 0) {
			break;
		}
		

		printf("GDB: ");
		fwrite(conn->recvbuf, conn->recvbuf_len, 1, stdout);
		printf("\n");

		gdb_packet_t packet = {0};
		GDBPacketInit(&packet);
		if (GDBPacketParse(&packet, conn->recvbuf, conn->recvbuf_len) < 0) {
			printf_log(LOG_INFO, "GDB: Failed to parse packet: %s\n", strerror(err));
		}
		GDBPacketFree(&packet);
	}




exit:
	err = close(conn->fd);
	if (err < 0) {
		printf_log(LOG_INFO, "GDB: Failed to close connection: %s\n", strerror(err));
	}


	// TODO: This might not be the best idea.
	// We are freeing the pthread structure before closing the thread.
	// Not sure if the pthread implementation requires us to keep that data
	free(conn);

	printf_log(LOG_DEBUG, "GDB: Closing listening thread: %d\n", conn->fd);

	return NULL;
}







int GDBServerWaitForConnection(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	// The thread will deallocate this
	server_connection_data_t *conn = calloc(1, sizeof(server_connection_data_t));

	printf_log(LOG_INFO, "GDB: Waiting for connection on port %d\n", gdb->port);
	conn->fd = accept(gdb->socketfd, NULL, NULL);
	if (conn->fd < 0) {
		printf_log(LOG_INFO, "GDB: Failed to accept connection: %s\n", strerror(conn->fd));
		return conn->fd;
	}


	int err = pthread_create(&conn->thread, NULL, GDBServerThreadLoop, conn);
	if (err < 0) {
		printf_log(LOG_INFO, "GDB: Failed to create connection thread: %s\n", strerror(err));
		return err;
	}


	return 0;
}

int GDBServerStart(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");
	assert(gdb->socketfd < 0 && "Called GDBServerStart with an active server socket");


	struct sockaddr_in serv_addr = {0};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(gdb->port);

	gdb->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (gdb->socketfd < 0) {
		printf_log(LOG_INFO, "GDB: Failed to open socket: %s\n", strerror(gdb->socketfd));
		return -1;
	}

	int err = bind(gdb->socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (err < 0) {
		printf_log(LOG_INFO, "GDB: Failed to bind socket: %s\n", strerror(err));
		return -1;
	}

	err = listen(gdb->socketfd, 5);
	if (err < 0) {
		printf_log(LOG_INFO, "GDB: Failed to call listen: %s\n", strerror(err));
		return -1;
	}

	return 0;
}

int GDBServerStop(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");
	assert(gdb->socketfd >= 0 && "Called GDBServerStop without first starting the server");

	// TODO: What happens if we close the server without closing the connection?
	int err = close(gdb->socketfd);
	gdb->socketfd = -1;
	if (err < 0) {
		printf_log(LOG_INFO, "GDB: Failed to close server: %s\n", strerror(err));
		return -1;
	}

	return 0;

}

int GDBServerNew(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	gdb->port = 1234;

	// Set invalid fd numbers
	gdb->socketfd = -1;

	return 0;
}

void GDBServerFree(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");
}