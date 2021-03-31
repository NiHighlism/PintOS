#include "userprog/syscall.h"
#include <lib/kernel/list.h>
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "process.h"

static void syscall_handler(struct intr_frame*);

extern bool running;

void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

static void syscall_handler(struct intr_frame* f UNUSED) {
  int* p = f->esp;

  is_valid_address(p);

  int system_call = *p;
  switch (system_call) {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      is_valid_address(p + 1);
      process_exit_helper(*(p + 1));
      break;

    case SYS_EXEC:
      is_valid_address(p + 1);
      is_valid_address(*(p + 1));
      f->eax = process_execute_helper(*(p + 1));
      break;

    case SYS_WAIT:
      is_valid_address(p + 1);
      f->eax = process_wait(*(p + 1));
      break;

    case SYS_WRITE:
      is_valid_address(p + 7);
      is_valid_address(*(p + 6));
      if (*(p + 5) == 1) {
        putbuf(*(p + 6), *(p + 7));
        f->eax = *(p + 7);
      }
  }
}

bool is_valid_address(const void* vaddr) {
  if (!is_user_vaddr(vaddr)) {
    process_exit_helper(EXIT_STATUS_FAIL);
    return false;
  }

  void* ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
  if (!ptr) {
    process_exit_helper(EXIT_STATUS_FAIL);
    return 0;
  }
  return true;
}
