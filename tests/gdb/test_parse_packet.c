#include<assert.h>
#include<string.h>
#include "../../src/gdb/packet.h"
#include "utils.h"


typedef struct packet_parse_test {
    char *str;
    gdb_packet_t packet;
} packet_parse_test_t;

static const packet_parse_test_t TEST_DATA[] = {
    {
        .str = "$#00",
        .packet = {
            .type = GDB_COMMAND_EMPTY,
        },
    },
    {
        .str = "$g#67",
        .packet = {
            .type = GDB_COMMAND_READ_REGISTER,
        },
    },
    {
        .str = "$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386#6a",
        .packet = {
            .type = GDB_COMMAND_QUERY_SUPPORTED,
        },
    }
};

static void test_sample_packets(void) {
    for (int i = 0; i < ARRAY_SIZE(TEST_DATA); i++) {
        gdb_packet_t packet = {0};
        packet_parse_test_t test = TEST_DATA[i];

        assert(GDBPacketInit(&packet) == 0);
        assert(GDBPacketParse(&packet, test.str, strlen(test.str)) >= 0);
        assert(GDBPacketEqual(&packet, &test.packet) == true);
        GDBPacketFree(&packet);
    }
}

int main() {
    test_sample_packets();
    return 0;
}