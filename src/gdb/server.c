#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include "../include/debug.h"
#include "./server.h"

static const size_t RECVBUF_START_SIZE = 64;


int GDBServerStart(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	struct sockaddr_in serv_addr = {0};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(gdb->port);

	gdb->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (gdb->socketfd < 0) {
		printf_log(LOG_DEBUG, "GDB: Failed to open socket with error: %s\n", strerror(gdb->socketfd));
		return -1;
	}

	int err = bind(gdb->socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (err < 0) {
		printf_log(LOG_DEBUG, "GDB: Failed to call bind with error: %s\n", strerror(err));
		return -1;
	}

	err = listen(gdb->socketfd, 5);
	if (err < 0) {
		printf_log(LOG_DEBUG, "GDB: Failed to call listen with error: %s\n", strerror(err));
		return -1;
	}

	return 0;
}

int GDBServerStop(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");
	assert(gdb->socketfd >= 0 && "Called GDBServerStop without first starting the server");

	int err = 0;
	if (gdb->connsockfd >= 0) {
		// TODO: There may be a bug here, the socket is still allocated after shutdown
		// But only when we have had a connection

		// err = shutdown(gdb->connsockfd, SHUT_RDWR);
		// if (err < 0) {
		//         printf_log(LOG_DEBUG, "GDB: Failed to shutdown connection with error: %s\n", strerror(err));
		// }

		// while (read(gdb->connsockfd) != 0);

		err = close(gdb->connsockfd);
		gdb->connsockfd = -1;
		if (err < 0) {
			printf_log(LOG_DEBUG, "GDB: Failed to close connection with error: %s\n", strerror(err));
		}
	}


	err = close(gdb->socketfd);
	gdb->connsockfd = -1;
	if (err < 0) {
		printf_log(LOG_DEBUG, "GDB: Failed to close connection with error: %s\n", strerror(err));
		return -1;
	}

	return 0;

}

int GDBServerWaitForConnection(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	printf_log(LOG_INFO, "GDB: Waiting for connection on port %d\n", gdb->port);
	gdb->connsockfd = accept(gdb->socketfd, NULL, NULL);
	if (gdb->connsockfd < 0) {
		printf_log(LOG_DEBUG, "GDB: Failed to accept connection with error: %s\n", strerror(gdb->connsockfd));
		return -1;
	}

	return 0;
}


int GDBServerNew(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	gdb->port = 1234;
	gdb->recvbuf_len = 0;
	gdb->recvbuf_alloc = RECVBUF_START_SIZE;
	gdb->recvbuf = malloc(gdb->recvbuf_alloc);
	if (gdb->recvbuf == NULL)  {
		return -1;
	}

	// Set invalid fd numbers
	gdb->socketfd = -1;
	gdb->connsockfd = -1;

	return 0;
}

void GDBServerFree(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");

	free(gdb->recvbuf);
}

// Returns the number of bytes read, or a negative number when an error occurs
int __GDBServerReadSocket(gdb_server_t *gdb) {
	assert(gdb != NULL && "NULL gdb struct");
	assert(gdb->connsockfd >= 0 && "Invalid connection socket number");


	char *buf = &gdb->recvbuf[gdb->recvbuf_len];
	size_t buf_len = gdb->recvbuf_alloc - gdb->recvbuf_len;
	ssize_t read_len = read(gdb->connsockfd, buf, buf_len);
	if (read_len < 0) {
		return -1;
	}

	gdb->recvbuf_len = gdb->recvbuf_len + read_len;

	// If we read all of the available size, try again after resizing,
	// because there might be more data.
	if ((size_t) read_len == buf_len) {
		size_t newalloc = gdb->recvbuf_alloc * 2;
		printf_log(LOG_DEBUG, "GDB: Resizing read buffer to: %zu\n", newalloc);
		char *newbuf = realloc(gdb->recvbuf, newalloc);
		if (newbuf == NULL) {
			return -1;
		}


		gdb->recvbuf = newbuf;
		gdb->recvbuf_alloc = newalloc;

		// Retry to read with the new buffer size;
		int rret = __GDBServerReadSocket(gdb);
		return (rret < 0) ? rret : (rret + read_len);
	}

	return read_len;
}