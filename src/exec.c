#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#include "internalcmd.h"

void exec_command(command_t *command)
{

    char *cmd = command->argv[0];
    if (is_internal(cmd)) jsh.last_exit_code = exec_internal(command);
    else if (fork() == 0)
    {
        execvp(cmd, command->argv);
        perror("jsh");
        free_command(command);
        jsh.last_exit_code = 127;
        exit(jsh.last_exit_code);
    }
    else
    {
        int status;
        waitpid(0, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }
}

