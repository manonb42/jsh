#include "jsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <wait.h>

#include "input.h"
#include "exec.h"

jsh_t jsh = {0};

void free_command(command_t *command)
{
    // returns true if command is in background
    for (int i = 0; i < command->argc; ++i)
        free(command->argv[i]);
    free(command->argv);
    free(command);
}

int main()
{

    rl_initialize();
    rl_outstream = stderr;

    while (1)
    {
        command_t *command;
        do {
            command = read_command();
        } while( command == NULL );

        exec_command(command);
        free_command(command);
    }

    return 0;
}
