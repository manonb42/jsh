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

bool free_command(command_t *command)
{
    // returns true if command is in background
    for (int i = 0; i < command->argc; ++i)
        free(command->argv[i]);
    free(command->argv);
    bool bg = command->bg;
    free(command);
    return bg;
}

int main()
{

    rl_initialize();
    rl_outstream = stderr;

    int nb_jobs = 0;
    while (1)
    {
        command_t *command = read_command(nb_jobs);
        while (waitpid(-1, NULL, WNOHANG) > 0)
            command->nb_jobs--;

        exec_command(command);
        nb_jobs = command->nb_jobs;

        if (free_command(command))
            exit(0);

        nb_jobs--;
    }

    return 0;
}