#ifndef EXEC_H_
#define EXEC_H_

#include "jsh.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void put_process_in_foreground(pid_t pid_grp);
void exec_command(command_t *);

#endif // EXEC_H_
