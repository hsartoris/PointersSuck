#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);


#include "threads/synch.h"

#define CLOSE_ALL -1
#define ERROR -1

#define NOT_LOADED 0
#define LOAD_SUCCESS 1
#define LOAD_FAIL 2

struct child_process {
  int pid;
  int load;
  bool wait;
  bool exit;
  int status;
  struct lock wait_lock;
  struct list_elem elem;
};

//struct child_process* addd_child_process (int pid);
struct child_process* get_child_process (int pid);
void remove_child_process (struct child_process *cp);
void remove_child_processes (void);

void process_close_file (int fd);

void syscall_init (void);


int is_valid_pointer(void* esp, uint8_t argc);
static int get_user (const uint8_t *uaddr);







#endif /* userprog/syscall.h */



