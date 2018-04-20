#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/file.h"

//this include for PHYS_BASE
#ifndef PHYS_BASE
#include "threads/vaddr.h"
#endif

static void syscall_handler (struct intr_frame *);

	void
syscall_init (void) 
{
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

	static void
syscall_handler (struct intr_frame *f) 
{
	int* call = (int*)f->esp;

	// some code for learning about interrupt frames
	intr_dump_frame(f); // spits out all the data in f
	// hex_dump (offset, buffer, size, bool of some kind?)
	hex_dump(f->esp, f->esp, (int)(PHYS_BASE - f->esp), true);
	// hopefully it works; just shut down after to avoid clutter
	shutdown_power_off();

	int* fd;
	const void* buff;
	unsigned* size;
	switch (*call) 
	{
		case SYS_HALT: //halt
			shutdown_power_off();

		case SYS_EXIT: //exit
			//TODO: status
			thread_exit();

		case SYS_EXEC: //exec
			
		case SYS_WAIT: //wait

		case SYS_CREATE: //create

		case SYS_REMOVE: //remove

		case SYS_OPEN: //open

		case SYS_FILESIZE: //filesize

		case SYS_READ: //read

		case SYS_WRITE: //write
			fd = (int*)(f->esp + 1);
			buff = (void*)(f->esp + 2);
			size = (unsigned*)(f->esp + 3);
			printf("%d\n", *fd);
			if (*fd == 1)
				putbuf(buff, *size);
			break;
		case SYS_SEEK: //seek

		case SYS_TELL: //tell

		case SYS_CLOSE: //close

		default:
			printf("aaaa\n");
	}
	printf ("system call %d!\n", *call);
	thread_exit ();
}
