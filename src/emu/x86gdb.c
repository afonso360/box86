#include <stdlib.h>

#include "box86context.h"
#include "debug.h"
#include "../gdb/server.h"

int InitX86GDBServer(box86context_t *context, int port) {
    if (context->gdb_server) {
        return 0;
    }

    context->gdb_server = calloc(1, sizeof(gdb_server_t));
    if (!context->gdb_server) {
        return 1;
    }

    if (GDBServerNew(context->gdb_server) < 0) {
        printf_log(LOG_INFO, "Failed to open gdb server\n");
        return 1;
    }

    if (GDBServerStart(context->gdb_server) < 0) {
        printf_log(LOG_INFO, "Failed to open gdb server\n");
        return 1;
    }

    if (GDBServerWaitForConnection(context->gdb_server) < 0) {
        printf_log(LOG_INFO, "Failed while waiting for a connection\n");
        return 1;
    }

    return 0;
}

void DeleteX86GDBServer(box86context_t *context) {
    if (!context->gdb_server) {
        return;
    }

    GDBServerStop(context->gdb_server);
    GDBServerFree(context->gdb_server);
    free(context->gdb_server);
    context->gdb_server = NULL;
}