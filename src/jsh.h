#ifndef JSH_H_
#define JSH_H_


typedef struct command_t {
    char **argv;
    int argc;
} command_t;

typedef struct jsh_t {
    int last_exit_code;
} jsh_t;

extern jsh_t jsh;


void free_command(command_t *command);

#endif // JSH_H_
