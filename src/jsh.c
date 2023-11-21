#include "jsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "input.h"
#include "exec.h"

jsh_t jsh = {0};

void free_command(command_t *command)
{
    for (int i = 0; i < command->argc; ++i)
        free(command->argv[i]);
    free(command->argv);
}

int main()
{

    rl_initialize();
    rl_outstream = stderr;

    while (1)
    {
        command_t *command = read_command();
        exec_command(command);
        free_command(command);
    }

    return 0;
}
