#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/inode.h"

//this include for PHYS_BASE
#ifndef PHYS_BASE
#include "threads/vaddr.h"
#endif

#define virt_bottom ((int *)0x0804ba68) //maybe supposed to be a void *

static void syscall_handler (struct intr_frame *);
int get_file_length(int fd);
struct file * get_file(int fd);
int read (int fd, void *buffer, unsigned size);

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
	//shutdown_power_off();
	printf("OH HOWDY%x\n", f->esp+1);
	int* fd;
	struct file* this_file;
	const void* buff;
	unsigned* size;
	switch (*call) 
	{
		case SYS_HALT: //halt
			printf("Halt!\n");
			shutdown_power_off();

		case SYS_EXIT: //exit
			//TODO: status
			printf("Exit!\n");
			thread_exit();

		case SYS_EXEC: //exec
			printf("Exec!\n");
		case SYS_WAIT: //wait
			printf("Wait!\n");
		case SYS_CREATE: //create
			printf("Create!\n");
		case SYS_REMOVE: //remove
			printf("Remove\n");
		case SYS_OPEN: //open
			printf("Remove\n");
		case SYS_FILESIZE: //filesize NOT YET TESTED
			printf("FileSize!\n");
			fd = (int*)(f->esp + 1);	//theoretically the file
			size = get_file_length(fd);
			printf("%d\n", size);
			break;
		case SYS_READ: //read
			printf("Read!\n");
			buff[1234];	//likely garbagio
			fd = (int*)(f->esp + 1);
			size = get_file_length(fd);
			read(fd, buff, size);
		case SYS_WRITE: //write
			printf("Write!\n");
			fd = (int*)(f->esp + 1);
			buff = (void*)(f->esp + 2);
			size = (unsigned*)(f->esp + 3);	//seems sketchy and wrong <3 cole
			printf("%d\n", &fd);
			printf("Hex of esp + 1 as int: %x\n", fd);
			printf("Hex of esp at end: %x\n", f->esp+178);

			if (*fd == 1)
				putbuf(buff, *size);
			break;
		case SYS_SEEK: //seek
			printf("Seek!\n");
		case SYS_TELL: //tell
			printf("Tell!\n");
		case SYS_CLOSE: //close
			printf("Close!\n");
		default:
			printf("We love vim <3\n");
	}
	printf ("system call %d!\n", *call);
	thread_exit ();
}


int get_file_length(int fd){
	struct inode* current_inode = inode_open(fd);
	return inode_length(current_inode);
}

struct file * get_file(int fd){
	struct inode* current_inode = inode_open(fd);
	return file_open(current_inode); 
}
//note buffer is in intq.h
int read (int fd, void *buffer, unsigned size){
	struct file* this_file = get_file(fd);
	int bytes_read = -1;
	bytes_read = file_read(this_file, buffer, size);
	return bytes_read;
}



/**************************************************************************
 *Grabbed these boys off some assembly site ??
 
 *cr2: control register
 *error: error, you fool
 *e$a-d$x: general purpose register
 *esi & edi: index registers for DS and ES respectively
 *esp: stack pointer B A B Y
 *ebp: access data on the stack
 *cs: code segment register, added to address during instruction "fetch"
 *ds: data segment register: added to address when accessing a memory operand
      that is not on the stack
 *es: extra segment register, also used in special instructions that span
      segments (ala: string copies)
 *ss: stack segment, added to address during stack access
 *************************************************************************/
