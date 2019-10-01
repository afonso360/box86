#ifndef __BOX86_GDB_PACKET_H_
#define __BOX86_GDB_PACKET_H_

#include<stddef.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>


typedef enum gdb_command_e {
	GDB_COMMAND_UNKOWN,
	GDB_COMMAND_EMPTY,
	GDB_COMMAND_READ_REGISTER,
	GDB_COMMAND_WRITE_REGISTER,
	GDB_COMMAND_QUERY_SUPPORTED,

	__GDB_COMMAND_MAX,
} gdb_command_t;


typedef enum gdb_value_type_e {
	GDB_VALUE_TYPE_BOOL,
	GDB_VALUE_TYPE_INT,

	__GDB_VALUE_TYPE_MAX,
} gdb_value_type_t;

typedef struct gdb_value_s {
	gdb_value_type_t type;
	union {
		bool _bool;
		int _int;
	};
} gdb_value_t;

typedef struct gdb_packet_s {
	gdb_command_t type;

	gdb_value_t *args;
	size_t args_len;
	size_t args_alloc;
} gdb_packet_t;

int GDBPacketInit(gdb_packet_t *packet);

// If the return code is negative, then an error has occured
// If the return code is positive, the number of bytes read is returned
ssize_t GDBPacketParse(gdb_packet_t *packet, char *buf, size_t len);

bool GDBPacketEqual(gdb_packet_t *left, gdb_packet_t *right);
void GDBPacketFree(gdb_packet_t *packet);



#endif //__BOX86_GDB_PACKET_H_
