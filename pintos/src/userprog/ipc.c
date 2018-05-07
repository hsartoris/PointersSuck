#include <list.h>
#include <stdio.h>
#include "lib/string.h"
#include "threads/synch.h"
#include "userprog/ipc.h"
#include "threads/malloc.h"

void read_release (struct write* w);
int write_accept (char* pipe, int id);

static struct list writes;
static struct list reads;

void ipc_init (void)
{
	list_init(&writes);
	list_init(&reads);
}

int ipc_read (char* pipe, int id)
{
#ifdef IPCDEBUG
	printf("ipc: attempting to read from pipe %s with id %d\n",
			pipe, id);
#endif
	// atempt to locate corresponding write
	int msg = write_accept (pipe, id);
	if (msg != WRITE_ACCEPT_FAILED)
		return msg;

	// no such write; add to waiters list and block process
	struct read* r = (struct read *) malloc(sizeof(struct read));
	sema_init(&r->read_sema, 0);
	r->id = id;
	r->pipe = pipe;
	list_push_back(&reads, &r->elem);
	sema_down(&r->read_sema);

	msg = write_accept(r->pipe, r->id);
	if (msg != WRITE_ACCEPT_FAILED)
	{
		list_remove(&r->elem);
		free(r);
		return msg;
	}
	NOT_REACHED();
}

int write_accept (char* pipe, int id)
{
#ifdef IPCDEBUG
	printf("ipc: attempting to accept message on pipe %s with id %d\n",
			pipe, id);
#endif
	struct list_elem *e;
	for (e = list_begin (&writes); e != list_end (&writes); e = list_next (e))
	{
		struct write* w = list_entry (e, struct write, elem);
		if (strcmp(w->pipe, pipe) == 0 && w->id == id)
		{
#ifdef IPCDEBUG
			printf("ipc: accepted message on pipe %s with id %d\n",
					pipe, id);
#endif
			list_remove(e);
			int msg = w->msg;
			free(w);
			return msg;
		}
	}
	return WRITE_ACCEPT_FAILED;
}

	

void ipc_write (char* pipe, int id, int msg)
{
	struct write * w = (struct write*) malloc(sizeof(struct write));
	w->id = id;
	w->pipe = pipe;
	w->msg = msg;
	list_push_back(&writes, &w->elem);
#ifdef IPCDEBUG
	printf("ipc: message registered on pipe %s from id %d with msg %d\n",
			pipe, id, msg);
#endif
	read_release(w);
}

void read_release (struct write* w)
{
#ifdef IPCDEBUG
	printf("ipc: attempting to free message on pipe %s with id %d and msg %d\n",
			w->pipe, w->id, w->msg);
#endif
	struct list_elem *e;
	for (e = list_begin (&reads); e != list_end (&reads); e = list_next (e))
	{
		struct read* r = list_entry (e, struct read, elem);
		if (strcmp(r->pipe, w->pipe) == 0 && r->id == w->id)
		{
#ifdef IPCDEBUG
			printf("ipc: freeing message on pipe %s with id %d and msg %d\n",
					w->pipe, w->id, w->msg);
#endif
			sema_up(&r->read_sema);
		}
	}
}
