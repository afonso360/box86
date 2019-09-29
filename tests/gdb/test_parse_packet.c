#include<assert.h>
#include<string.h>
#include "../../src/gdb/packet.h"


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


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
    }
};

static void test_sample_packets(void) {
    for (int i = 0; i < ARRAY_SIZE(TEST_DATA); i++) {
        gdb_packet_t packet = {0};
        packet_parse_test_t test = TEST_DATA[i];
        assert(GDBPacketParse(&packet, test.str, strlen(test.str)) >= 0);
        assert(memcmp(&packet, &test.packet, sizeof(gdb_packet_t)) == 0);
    }
}

int main() {
    test_sample_packets();
    return 0;
}