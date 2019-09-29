#include<assert.h>
#include<stdlib.h>
#define _GNU_SOURCE
#include<string.h>
#include<stdint.h>

#include "./packet.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static int min(int a, int b) {
	return (a < b) ? a : b;
}


// String as it should appear on the packet
static const char *GDB_COMMAND_STRING[] = {
	[GDB_COMMAND_READ_REGISTER] = "g",
	[GDB_COMMAND_WRITE_REGISTER] = "G",
};


static gdb_command_t GDBPacketParseCommand(char *buf, size_t len) {
	assert(buf != NULL && "NULL buffer passed");

	if (len == 0) {
		return GDB_COMMAND_EMPTY;
	}

	for (size_t i = 0; i < ARRAY_SIZE(GDB_COMMAND_STRING); i++) {
		const char *cmd = GDB_COMMAND_STRING[i];
		if (cmd == NULL) {
			continue;
		}

		int ret = strncmp(buf, cmd, min(len, strlen(cmd)));
		if (ret == 0) {
			return i;
		}
	}

	return GDB_COMMAND_UNKOWN;
}


static int GDBPacketParseData(gdb_packet_t *packet, char *buf, size_t len) {
	assert(packet != NULL && "NULL packet struct");
	assert(buf != NULL && "NULL buffer passed");

	packet->type = GDBPacketParseCommand(buf, len);

	return 0;
}

static int GDBPacketValidate(char *buf, size_t len) {
	assert(buf != NULL && "NULL buffer passed");
	assert(len >= 4 && "Invalid packet size passed");

	// Convert the checksum to a string and parse it with strtol
	char *checksum_buf = &buf[len - 2];
	char checksum_str[] = {
		checksum_buf[0],
		checksum_buf[1],
		'\0'
	};
	long checksum = strtol(checksum_str, NULL, 16);


	char *packet_data_start = buf + 1;
	char *packet_data_end = ( buf + len ) - 3;
	size_t packet_data_len = (packet_data_end - packet_data_start);

	uint8_t calc_checksum = 0;
	for (size_t i = 0; i < packet_data_len; i++) {
		char c = packet_data_start[i];
		calc_checksum = (calc_checksum + c) % 256;
	}

	if (checksum != calc_checksum) {
		// Disable checksum validation when fuzzing.
		// This is just so that the fuzzer can be a bit more efficient
#ifdef FUZZING
		return 0;
#else
		return -1;
#endif
	}

	return 0;
}



int GDBPacketParse(gdb_packet_t *packet, char *buf, size_t len) {
	assert(packet != NULL && "NULL packet struct");
	assert(buf != NULL && "NULL buffer passed");

	char *packet_start = memchr(buf, '$', len);
	if (packet_start == NULL) {
		return -1;
	}

	size_t preamble_len = packet_start - buf;
	size_t packet_start_len = len - preamble_len;
	char *packet_data_end = memchr(packet_start, '#', packet_start_len);
	if (packet_data_end == NULL) {
		return -1;
	}


	// Packet size, is the sum of
	//   starting delimiter (1)
	//   packet_data_len (variable)
	//   final delimiter (1)
	//   2 checksum bytes (2)
	char *packet_data_start = packet_start + 1;
	size_t packet_data_len = (packet_data_end - packet_data_start);
	size_t packet_len = packet_data_len + 4;

	// Check if data has all of the packet data
	if (preamble_len + packet_len > len) {
		return -1;
	}


	if (GDBPacketValidate(packet_start, packet_len) < 0) {
		return -1;
	}

	if (GDBPacketParseData(packet, packet_start, packet_len) < 0) {
		return -1;
	}

	return 0;
}
