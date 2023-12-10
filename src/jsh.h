#ifndef JSH_H_
#define JSH_H_
#include <stdbool.h>
#include "vector.h"


typedef struct command_t {
    bool bg; // true if the cmd has been called with '&'
    char **argv;
    int argc;
    int nb_jobs;
} command_t;

void free_command(command_t *command);

typedef struct process_t {
    int pid;
} process_t;

typedef struct vector process_v;

typedef struct jsh_t
{
    int last_exit_code;
    int pid_to_stop;
    process_v processes;
} jsh_t;

extern jsh_t jsh;


#endif // JSH_H_
