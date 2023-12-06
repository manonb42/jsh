#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"

int pwd();
int cd(char *);
int quit(int, command_t *);
int showLastReturnCode();
bool is_internal(char *);
int exec_internal(command_t *);
void exec_external(command_t *command);

#endif
