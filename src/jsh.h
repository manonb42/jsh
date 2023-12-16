#ifndef JSH_H_
#define JSH_H_
#include <stdbool.h>
#include "vector.h"


typedef enum command_redir_type_t {
  R_NONE,
  R_INPUT,
  R_NO_CLOBBER,
  R_CLOBBER,
  R_APPEND,
} command_redir_type_t;


typedef struct command_redir_t {
  command_redir_type_t type;
  char *path;
} command_redir_t;


typedef struct command_t {
    bool bg;

    command_redir_t stdin;
    command_redir_t stdout;
    command_redir_t stderr;

    char **argv;
    int argc;
} command_t;

void free_command(command_t *command);


typedef enum process_state_t {
   P_NONE,
   P_RUNNING,
   P_STOPPED,
   P_DONE,
   P_KILLED,
   P_DETACHED
} process_state_t;

typedef struct process_t {
    int pid;
    process_state_t current_state;
    process_state_t notified_state;
} process_t;

typedef struct vector process_v;

typedef struct jsh_t
{
    int last_exit_code;
    process_v processes;
} jsh_t;

extern jsh_t jsh;


#endif // JSH_H_
