#include "userprog/handlers.h"
#include <debug.h>
#include <stdio.h>
#include <stdbool.h>
#include "lib/kernel/list.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

void SYSCALL_exit_handler(int status) {
  struct list_elem* e;

  for (e = list_begin(&thread_current()->parent->process_children);
       e != list_end(&thread_current()->parent->process_children); e = list_next(e)) {
    struct child_process* f = list_entry(e, struct child_process, elem);
    if (f->tid == thread_current()->tid) {
      f->old = true;
      f->exit_status = status;
    }
  }

  thread_current()->exit_status = status;

  if (thread_current()->parent->tid_wait == thread_current()->tid)
    sema_up(&thread_current()->parent->child_process_lock);

  thread_exit();
}

int SYSCALL_execute_handler(char* file_name) {
  lock_acquire(&global_filesystem_lock);

  char* filename_temp = malloc(strlen(file_name) + 1);
  strlcpy(filename_temp, file_name, strlen(file_name) + 1);

  char* save_ptr;
  filename_temp = strtok_r(filename_temp, " ", &save_ptr);

  struct file* f = filesys_open(filename_temp);

  if (f == NULL) {
    lock_release(&global_filesystem_lock);
    return -1;
  } else {
    file_close(f);
    lock_release(&global_filesystem_lock);
    return process_execute(file_name);
  }
}

int SYSCALL_write_handler(int fd, const void* buffer, unsigned size) {
  if (fd == STDOUT_FD) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

bool is_valid_address(const void* vaddr) {
  if (!is_user_vaddr(vaddr) || !pagedir_get_page(thread_current()->pagedir, vaddr)) {
    SYSCALL_exit_handler(EXIT_STATUS_FAIL);
    return false;
  }

  return true;
}