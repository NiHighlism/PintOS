#include "userprog/syscall.h"
#include "list.h"
#include "process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/handlers.h"
#include <stdio.h>
#include <syscall-nr.h>

static void syscall_handler(struct intr_frame*);

extern bool running;

void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

static void syscall_handler(struct intr_frame* f) {
  int* p = f->esp;

  is_valid_address(p);

  int system_call = *p;
  switch (system_call) {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      is_valid_address(p + 1);
      SYSCALL_exit_handler(*(p + 1));
      break;

    case SYS_EXEC:
      is_valid_address(p + 1);
      is_valid_address(*(p + 1));
      f->eax = SYSCALL_execute_handler(*(p + 1));
      break;

    case SYS_WAIT:
      is_valid_address(p + 1);
      f->eax = SYSCALL_wait_handler(*(p + 1));
      break;

    case SYS_CREATE:
      is_valid_address(p + 5);
      is_valid_address(*(p + 4));

      f->eax = SYSCALL_create_handler(*(p + 4), *(p + 5));
      break;

    case SYS_REMOVE:
      is_valid_address(p + 1);
      is_valid_address(*(p + 1));

      f->eax = SYSCALL_remove_handler(*(p + 1));
      break;

    case SYS_OPEN:
      is_valid_address(p + 1);
      is_valid_address(*(p + 1));

      f->eax = SYSCALL_open_handler(*(p + 1));
      break;

    case SYS_WRITE:
      is_valid_address(p + 7);
      is_valid_address(*(p + 6));

      f->eax = SYSCALL_write_handler(*(p + 5), *(p + 6), *(p + 7));
      break;
  }
}
