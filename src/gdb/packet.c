#include<assert.h>
#include<stdlib.h>
#include<stdint.h>

#include "utils.h"
#include "./packet.h"

static const int DEFAULT_ARGS_SIZE = 4;

// String as it should appear on the packet
static const char *GDB_COMMAND_STRING[] = {
	[GDB_COMMAND_READ_REGISTER] = "g",
	[GDB_COMMAND_WRITE_REGISTER] = "G",
	[GDB_COMMAND_QUERY_SUPPORTED] = "qSupported",
};

// Stores a list of the argument types that should be parsed on each command
static gdb_value_type_t **GDB_COMMAND_ARGUMENTS[] = {
	[GDB_COMMAND_READ_REGISTER] = (gdb_value_type_t *[]) { NULL },
	[GDB_COMMAND_WRITE_REGISTER] = (gdb_value_type_t *[]){
		&(gdb_value_type_t) { GDB_VALUE_TYPE_INT }
	},
	[GDB_COMMAND_QUERY_SUPPORTED] = (gdb_value_type_t *[]) { NULL },

};


static int GDBValueParse(gdb_value_t *val, gdb_value_type_t type, char *buf, size_t len) {
	assert(val != NULL && "NULL val struct");
	assert(buf != NULL && "NULL buffer passed");

	val->type = type;

	switch(type) {
	GDB_VALUE_TYPE_BOOL:
		val->_bool = true;
	GDB_VALUE_TYPE_INT:
		val->_int = 2;
	default:
		assert(0 && "Unkown value type");
	}

	return 0;
}


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

static int GDBPacketParseArgs(gdb_packet_t *packet, char *buf, size_t len) {
	assert(packet != NULL && "NULL packet struct");
	assert(buf != NULL && "NULL buffer passed");

	gdb_value_type_t **parse_config = GDB_COMMAND_ARGUMENTS[packet->type];
	if (parse_config == NULL || parse_config[0] == NULL) {
		return 0;
	}

	// TODO: Clean this up
	for (size_t i = 0; i < ARRAY_SIZE(parse_config); i++ ) {
		gdb_value_type_t type = *(parse_config[i]);
		gdb_value_t val = {0};

		int res = GDBValueParse(&val, type, buf, len);
		if (res < 0) {
			return -1;
		}

		// TODO: GDBPacketAppendArg()
		buf = &buf[res];
		len -= res;
	}

	return 0;
}


static int GDBPacketParseData(gdb_packet_t *packet, char *buf, size_t len) {
	assert(packet != NULL && "NULL packet struct");
	assert(buf != NULL && "NULL buffer passed");

	packet->type = GDBPacketParseCommand(buf, len);

	const char *command_string = GDB_COMMAND_STRING[packet->type];
	if (command_string == NULL) {
		return 0;
	}
	
	buf = &buf[strlen(command_string)];
	len -= strlen(command_string);

	if (GDBPacketParseArgs(packet, buf, len) < 0) {
		return -1;
	}

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


// TODO: return bytes read
// TODO: Change return codes
ssize_t GDBPacketParse(gdb_packet_t *packet, char *buf, size_t len) {
// int GDBPacketParse(gdb_packet_t *packet, char *buf, size_t len) {
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

	// TODO: This is duplicated in GDBPacketValidate
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

	if (GDBPacketParseData(packet, packet_data_start, packet_data_len) < 0) {
		return -1;
	}

	return 0;
}

bool GDBPacketEqual(gdb_packet_t *left, gdb_packet_t *right) {
	assert(left != NULL && "NULL packet struct");
	assert(right != NULL && "NULL packet struct");

	return
		left->type == right->type &&
		left->args_len == right->args_len &&
		memcmp(left->args, right->args, right->args_len * sizeof(gdb_value_t)) == 0;
}


int GDBPacketInit(gdb_packet_t *packet) {
	assert(packet != NULL && "NULL packet struct");

	packet->type = GDB_COMMAND_UNKOWN;
	packet->args = calloc(DEFAULT_ARGS_SIZE, sizeof(gdb_value_t));
	packet->args_alloc = DEFAULT_ARGS_SIZE;
	packet->args_len = 0;

	return 0;
}


void GDBPacketFree(gdb_packet_t *packet) {
	assert(packet != NULL && "NULL packet struct");
	
	packet->type = GDB_COMMAND_UNKOWN;

	if (packet->args != NULL) {
		free(packet->args);
		packet->args = NULL;
		packet->args_len = 0;
		packet->args_alloc = 0;
	}

	return;
}