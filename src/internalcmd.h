#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"
#include "stdbool.h"


bool is_internal(char *name);
int exec_internal(command_t *command);

#endif
