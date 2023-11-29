#ifndef INTERNALCMD_H_
#define INTERNALCMD_H_

#include "jsh.h"

int pwd();
int cd(char *);
int quit(int, command_t *);
int showLastReturnCode();
#endif