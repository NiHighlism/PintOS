#include <stdbool.h>

#define STDOUT_FD 1

void SYSCALL_exit_handler(int status);
int SYSCALL_execute_handler(char* file_name);
int SYSCALL_write_handler(int fd, const void* buffer, unsigned size);

// Helper functions
bool is_valid_address(const void* addr);
