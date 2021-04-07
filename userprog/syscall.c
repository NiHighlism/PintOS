#include "userprog/syscall.h"
#include "list.h"
#include "process.h"
#include "devices/shutdown.h"
#include "filesys/off_t.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/handlers.h"
#include <stdio.h>
#include <syscall-nr.h>

static void syscall_handler(struct intr_frame*);

extern bool running;

/* Registers a new interrupt with the code 0x30, to be handled by the `syscall_handler` function */
void syscall_init(void) { intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall"); }

/*
  Each syscall is handled in two parts - memory checking and syscall handling

  Memory checking is done by checking if an address on the stack is a valid
  address. Since PintOS reserves memory above PHYS_BASE as kernel memory, user processes
  can't access it. This is the first check. The other check if making sure that the address
  belongs to a valid page from the processes page directory

  Once the addresses of all stack members are checked, a handler function is called, whose return
  value (if any) is stored in the eax register of the interrupt frame. The kernel returns it to the
  caller function in the user process once control returns to user mode from kernel mode

  All  handler functions are defined in `userprog/handlers.c`
*/
static void syscall_handler(struct intr_frame* f) {
  int* p = f->esp;

  is_valid_address((int*)p);

  int system_call = *p;
  switch (system_call) {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT:
      is_valid_address((int*)p + 1);
      SYSCALL_exit_handler((int)*(p + 1));
      break;

    case SYS_EXEC:
      is_valid_address((int*)p + 1);
      is_valid_address((int*)*(p + 1));
      f->eax = SYSCALL_execute_handler((char*)*(p + 1));
      break;

    case SYS_WAIT:
      is_valid_address((int*)p + 1);
      f->eax = SYSCALL_wait_handler((tid_t) * (p + 1));
      break;

    case SYS_CREATE:
      is_valid_address((int*)p + 5);
      is_valid_address((int*)*(p + 4));

      f->eax = SYSCALL_create_handler((const char*)*(p + 4), (off_t) * (p + 5));
      break;

    case SYS_REMOVE:
      is_valid_address((int*)p + 1);
      is_valid_address((int*)*(p + 1));

      f->eax = SYSCALL_remove_handler((const char*)*(p + 1));
      break;

    case SYS_OPEN:
      is_valid_address((int*)p + 1);
      is_valid_address((int*)*(p + 1));

      f->eax = SYSCALL_open_handler((const char*)*(p + 1));
      break;

    case SYS_FILESIZE:
      is_valid_address((int*)p + 1);

      f->eax = SYSCALL_filesize_handler((int)*(p + 1));
      break;

    case SYS_READ:
      is_valid_address((int*)p + 7);
      is_valid_address((int*)*(p + 6));

      f->eax = SYSCALL_read_handler((int)*(p + 5), (void*)*(p + 6), (unsigned int)*(p + 7));
      break;

    case SYS_WRITE:
      is_valid_address((int*)p + 7);
      is_valid_address((int*)*(p + 6));

      f->eax = SYSCALL_write_handler((int)*(p + 5), (const void*)*(p + 6), (unsigned int)*(p + 7));
      break;

    case SYS_SEEK:
      is_valid_address((int*)p + 5);

      SYSCALL_seek_handler((int)*(p + 4), (off_t) * (p + 5));
      break;

    case SYS_TELL:
      is_valid_address((int*)p + 1);

      SYSCALL_tell_handler((int)*(p + 1));
      break;

    case SYS_CLOSE:
      is_valid_address((int*)p + 1);

      SYSCALL_close_handler((int)*(p + 1));
      break;
  }
}
