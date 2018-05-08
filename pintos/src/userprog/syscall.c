#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/file.h"
#include "filesys/inode.h"

#include <user/syscall.h>
#include "threads/malloc.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"

//this include for PHYS_BASE
#ifndef PHYS_BASE
#include "threads/vaddr.h"
#endif

#define virt_bottom ((void *)0x0804ba68) //maybe supposed to be a void *

static void syscall_handler (struct intr_frame *);
int get_file_length(int fd);
struct file * get_file(int fd);
int read (int fd, void *buffer, unsigned size);
void get_arg (struct intr_frame *f, int *arg, int n);
int open (const char *file);
int user_to_kernel_ptr (const void *vaddr);
int write (int fd, const void *buffer, unsigned size);
void check_valid_ptr (const void *vaddr);
void check_valid_buffer (void* buffer, unsigned size);
int add_file (struct file *f);
struct file* process_get_file (int fd);
static bool is_valid_string(void * str);


struct process_file {
  struct file *file;
  int fd;
  struct list_elem elem;
};

	void
syscall_init (void) 
{
	intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

	static void
syscall_handler (struct intr_frame *f) 
{
	int* call = (int*)f->esp;
	
	void* arg[10];	//arbitrary max


	int* fd;
	struct file* this_file;
	const void* buff;
	unsigned* size;
	switch (*call) 
	{
	
		case SYS_HALT: //halt
			shutdown_power_off();
			break;
		case SYS_EXIT: //exit
			get_arg(f, &arg[0], 1);
			exit(*(int*)arg[0]);
			break;
		case SYS_EXEC: //exec
			if (!is_valid_pointer(f->esp + 4, 4))
				return -1;
			get_arg(f, &arg[0], 1);
			f->eax = exec(*(const char **) arg[0]);
	
			break;
		case SYS_WAIT: //wait
			get_arg(f, &arg[0], 1);
			f->eax = wait(arg[0]);
			break;
		case SYS_CREATE: //create
			if (!is_valid_pointer(f->esp+4, 4)||
				!is_valid_string(*(char **)(f->esp +4)) || 
				!is_valid_pointer(f->esp+8, 4)){
				return -1;
			}
			get_arg(f, &arg[0], 3);
			f->eax = create(*(char **)(f->esp+4), *(int *)(f->esp +8));
			return 0;
			break;
		case SYS_REMOVE: //remove
			if (!is_valid_pointer(f->esp +4, 4) ||
				!is_valid_string(*(char **)(f->esp+4))){
				return -1;
			}
			f->eax = remove(*(char **)(f->esp + 4));
			return 0;
			break;
		case SYS_OPEN: //open
			if (!is_valid_pointer(f->esp+4,4))
				return -1;
			get_arg(f, &arg[0], 1);
			arg[0] = user_to_kernel_ptr((const void *) arg[0]);
			if (arg[0] == -1) {
				f->eax = -1;
				break;
			}
			f->eax = open(*(char **) arg[0]);
			break;
		case SYS_FILESIZE: //filesize NOT YET TESTED
			get_arg(f, &arg[0], 1);
			f->eax = get_file_length(arg[0]);
			break;
		case SYS_READ: //read
			get_arg(f, &arg[0], 3);
			if(!is_valid_pointer(f->esp +4, 12)){
				return -1;
			}
			check_valid_buffer((void *) arg[1], *(unsigned*) arg[2]);
			f->eax = read(*(int*)arg[0], *(char **) arg[1],
				       	*(unsigned*) arg[2]);
			break;
	
		case SYS_WRITE: //write
			get_arg(f, &arg[0], 3);
			if(!is_valid_pointer(f->esp +4, 12)){
				return -1;
			}		
			check_valid_buffer((void *) arg[1], *(unsigned*) arg[2]);
			f->eax = write(*(int*)arg[0], *(char **) arg[1], 
					*(unsigned*) arg[2]);
			break;	

		case SYS_SEEK: //seek
			get_arg(f, &arg[0], 2);
			struct file* file_entry = process_get_file(*(int*)arg[0]);
			if (file_entry != NULL)
				file_seek(file_entry, *(unsigned*) arg[1]);
			break;
		case SYS_TELL: //tell
			get_arg(f, &arg[0], 1);
			file_entry = process_get_file(*(int *)arg[0]);
			if (file_entry == NULL)
				f->eax = -1;
			else
				f->eax = file_tell(file_entry);
			break;
		case SYS_CLOSE: //close
			break;
		default:
			printf("We love vim <3\n");
	}
	
}

int get_file_length(int fd){
	struct file *f = process_get_file(fd);
	if(!f){
		return ERROR;
	}
	return file_length(f);
}

struct file * get_file(int fd){
	struct inode* current_inode = inode_open(fd);
	return file_open(current_inode); 
}

int add_file (struct file *f){
  struct process_file *pf = malloc(sizeof(struct process_file));
  pf->file = f;
  pf->fd = thread_current()->fd;
  thread_current()->fd++;
  list_push_back(&thread_current()->file_list, &pf->elem);
  return pf->fd;
}

struct file* process_get_file (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *e;

  for (e = list_begin (&t->file_list); e != list_end (&t->file_list);
       e = list_next (e))
        {
          struct process_file *pf = list_entry (e, struct process_file, elem);
          if (fd == pf->fd)
	    {
	      return pf->file;
	    }
        }
  return NULL;
}

bool create (const char *file_name, unsigned start_size){
	return filesys_create(file_name, start_size);
}

bool remove (const char *file_name){
	return filesys_remove (file_name);
}



int read (int fd, void *buffer, unsigned size){
	if(fd == STDIN_FILENO){
		unsigned i;
		uint8_t* local_buffer = (uint8_t *) buffer;
		for(i=0; i<size; i++){
			local_buffer[i] = input_getc();
		}
		
		return size;
	}
	else if(process_get_file(fd) != NULL){
                struct file *file = process_get_file(fd);
                return (int)file_read(file, buffer, size);
        }
	return -1;
	}
int write (int fd, const void *buffer, unsigned size){
	if (fd == STDOUT_FILENO){
		putbuf(buffer, size);
		return size;
	}
	else if(process_get_file(fd) != NULL){
		struct file *file = process_get_file(fd);
		return (int)file_write(file, buffer, size);
	}
	return -1;
}

int wait (pid_t pid){
	return process_wait(pid);
}

int open (const char *file){
	if (file == NULL)
		return -1;
	struct file *f = filesys_open(file);
	if(f == NULL)
		return -1;
	
  	struct process_file *pf = malloc(sizeof(struct process_file));
  	pf->file = f;
  	pf->fd = thread_current()->fd++;
  	list_push_back(&thread_current()->file_list, &pf->elem);
	return pf->fd;
}

int exec(const char *cmd_line){
	int pid = process_execute(cmd_line);
	struct child_process* cp = get_child_process(pid);
	ASSERT(cp);
	//return pid;
	while (cp->load == NOT_LOADED){
		barrier();
	}
	if(cp->load == LOAD_FAIL){
		return ERROR;
	}
	return pid;
}

void exit (int status){
	thread_current ()->status = status;
	thread_exit();
}


int user_to_kernel_ptr(const void *vaddr){
	if (!is_user_vaddr(vaddr) || vaddr < virt_bottom)
		return -1;
	if (vaddr > PHYS_BASE)
		exit(-1);
	void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
	if(ptr == NULL)
		return -1;
	return (int) ptr;
}



struct child_process* get_child_process (int pid){
	struct thread *t = thread_current();
	struct list_elem *e;


	for (e = list_begin(&t->child_list); e != list_end(&t->child_list); e = list_next(e)){
		struct child_process *cp = list_entry(e, struct child_process, elem);
		if(pid == cp->pid){
			return cp;
		}
	}
	return NULL;
}

void remove_child_process(struct child_process *cp){
	list_remove(&cp->elem);
	free(cp);
}

void remove_child_processes(void){
	struct thread *t = thread_current();
	struct list_elem *next, *e = list_begin(&t->child_list);

	while(e != list_end(&t->child_list)){
		next = list_next(e);
		struct child_process *cp = list_entry(e, struct child_process, elem);
		list_remove(&cp->elem);
		free(cp);
		e = next;
	}
		
}

static int get_user (const uint8_t* uaddr)
{
	int result;
	asm ("movl $1f, %0; movzbl %1, %0; 1:"
			: "=&a" (result) : "m" (*uaddr));
	return result;
}

static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if(!is_user_vaddr(udst))
    return false;
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

int is_valid_pointer(void* esp, uint8_t argc)
{
	uint8_t i;
	for (i = 0; i < argc; ++i)
	{
		if (get_user (((uint8_t*)esp)+i) == -1)
		{
			return -1;
		}
	}
	return 1;
}

static bool is_valid_string(void * str)
{
  int ch=-1;
  while((ch=get_user((uint8_t*)str++))!='\0' && ch!=-1);
    if(ch=='\0')
      return true;
    else
      return false;
}


void check_valid_ptr(const void *vaddr){
	if (!is_user_vaddr(vaddr) || vaddr < virt_bottom){
		exit(ERROR);
	}
}

void check_valid_buffer(void* buffer, unsigned size){
	unsigned i;
	char* local_buffer = (char *) buffer;
	for(i=0; i<size; i++){
		check_valid_ptr((const void*) local_buffer);
		local_buffer++;
	}
}

void get_arg (struct intr_frame *f, int *arg, int n){
	int i, *ptr;
	int wlen = sizeof(void*);
	for(i = 0; i < n; i++) {
		arg[i] = f->esp + ((i+1) * wlen);
	}

}


void process_close_file (int fd)
{
  struct thread *t = thread_current();
  struct list_elem *next, *e = list_begin(&t->file_list);

  while (e != list_end (&t->file_list))
    {
      next = list_next(e);
      struct process_file *pf = list_entry (e, struct process_file, elem);
      if (fd == pf->fd || fd == CLOSE_ALL)
	{
	  file_close(pf->file);
	  list_remove(&pf->elem);
	  free(pf);
	  if (fd != CLOSE_ALL)
	    {
	      return;
	    }
	}
      e = next;
    }
}

/**************************************************************************
 
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
 */
