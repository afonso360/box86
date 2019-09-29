#include<assert.h>
#include<stdio.h>
#include "../../src/gdb/server.h"


int box86_log = 3;    // log level
int dlsym_error = 0;  // log dlsym error
int trace_xmm = 0;    // include XMM reg in trace?
FILE* ftrace = 0;


// Guarantees we close our sockets properly
static void test_server_cleanly_shuts_down(void) {
    gdb_server_t gdb = {0};

    assert(GDBServerNew(&gdb) == 0);

	assert(GDBServerStart(&gdb) == 0);
	assert(GDBServerStop(&gdb) == 0);

    assert(GDBServerStart(&gdb) == 0);
	assert(GDBServerStop(&gdb) == 0);

	GDBServerFree(&gdb);
}

int main() {
    test_server_cleanly_shuts_down();
    return 0;
}