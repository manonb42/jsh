#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"

bool is_internal(char *);
int exec_internal(command_t *);
void exec_external(command_t *command);

#endif
