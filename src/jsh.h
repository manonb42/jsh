#ifndef JSH_H_
#define JSH_H_
#include <stdbool.h>


typedef struct command_t {
    bool bg; // true if the cmd has been called with '&'
    char **argv;
    int argc;
    int nb_jobs;
} command_t;

void free_command(command_t *command);
typedef struct jsh_t
{
    int last_exit_code;
    int pid_to_stop;
} jsh_t;

extern jsh_t jsh;


#endif // JSH_H_
