#ifndef JSH_H_
#define JSH_H_
#include <stdbool.h>
#include "vector.h"


typedef struct command_t {
    bool bg; // true if the cmd has been called with '&'
	bool is_redir; //true if cmd is part of a redirection
	char *redir; //type of redirection
	char *fic; //fic part of the redirection
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
