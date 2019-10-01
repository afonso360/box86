#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../../src/gdb/packet.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	gdb_packet_t packet = {0};
	GDBPacketInit(&packet);
	GDBPacketParse(&packet, data, size);
	GDBPacketFree(&packet);
	return 0;
}