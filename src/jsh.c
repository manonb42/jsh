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

    int nb_jobs = 0;
    while (1)
    {
        command_t *command;
        do {
            command = read_command(0);
        } while( command == NULL );

        while (waitpid(-1, NULL, WNOHANG) > 0)
-            command->nb_jobs--;

        exec_command(command);
        nb_jobs = command->nb_jobs;

        bool bg = command->bg;
        free_command(command);
        if (bg)
            exit(0);

        nb_jobs--;
    }

    return 0;
}