#ifndef __BOX86_GDB_PACKET_H_
#define __BOX86_GDB_PACKET_H_

#include<stddef.h>


typedef enum gdb_command {
	GDB_COMMAND_UNKOWN,
	GDB_COMMAND_EMPTY,
	GDB_COMMAND_READ_REGISTER,
	GDB_COMMAND_WRITE_REGISTER,

	__GDB_COMMAND_MAX,
} gdb_command_t;


typedef struct gdb_packet {
	gdb_command_t type;
} gdb_packet_t;

int GDBPacketParse(gdb_packet_t *packet, char *buf, size_t len);



#endif //__BOX86_GDB_PACKET_H_
