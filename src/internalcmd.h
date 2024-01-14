#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"

bool is_internal(char *);
void exec_internal(command_t *command, job_t *job);

#endif
