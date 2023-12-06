#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"
#include "stdbool.h"

int exec_pwd();
int exec_cd(char *);
int exec_exit(int, command_t *);
int exec_show_last_return_code();
bool is_internal(char *);
int exec_internal(command_t *);
void exec_external(command_t *command);

#endif
