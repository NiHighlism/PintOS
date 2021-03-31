#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
void syscall_init(void);

bool is_valid_address(const void*);

#endif /* userprog/syscall.h */
