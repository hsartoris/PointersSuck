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

	int* fd;
	struct file* this_file;
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

		case SYS_FILESIZE: //filesize NOT YET TESTED
			fd = (int*)(f->esp + 1);	//theoretically the file
			size = get_file_length(fd);
			printf("%d\n", size);
			break;
		case SYS_READ: //read
			buff[1234];	//likely garbagio
			fd = (int*)(f->esp + 1);
			size = get_file_length(fd);
			read(fd, buff, size);
		case SYS_WRITE: //write
			fd = (int*)(f->esp + 1);
			buff = (void*)(f->esp + 2);
			size = (unsigned*)(f->esp + 3);	//seems sketchy and wrong <3 cole
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