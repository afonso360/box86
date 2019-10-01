#ifndef __X86GDB_H_
#define __X86GDB_H_

typedef struct box86context_s box86context_t;
typedef struct gdb_server_s gdb_server_t;

int InitX86GDBServer(box86context_t *context, int port);
void DeleteX86GDBServer(box86context_t *context);

#endif //__X86GDB_H_