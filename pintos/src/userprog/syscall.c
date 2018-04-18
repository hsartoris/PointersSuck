#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

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

		case SYS_SEEK: //seek

		case SYS_TELL: //tell

		case SYS_CLOSE: //close

		default:
			printf("go fuck yourself\n");

	}
	printf ("system call %d!\n", *call);
	thread_exit ();
}
