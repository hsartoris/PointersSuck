#ifndef USERPROG_IPC_H
#define USERPROG_IPC_H

#define WRITE_ACCEPT_FAILED -10000

struct write
{
	int id;
	char* pipe;
	int msg;
	struct list_elem elem;
};

struct read
{
	int id;
	char* pipe;
	struct semaphore read_sema;
	struct list_elem elem;
};

void ipc_init (void);
int ipc_read (char* pipe, int id);
void ipc_write (char* pipe, int id, int msg);

#endif
