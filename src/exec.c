#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

#include "internalcmd.h"

void exec_command(command_t *command)
{

    char *cmd = command->argv[0];

    if (strcmp(cmd, "?") == 0)
    {
        printf("%d\n", jsh.last_exit_code);
    }
    else if (strcmp(cmd, "pwd") == 0)
        pwd();
    else if (strcmp(cmd, "cd") == 0)
        cd(command->argv[1]);
    else if (strcmp(cmd, "exit") == 0)
        quit(jsh.last_exit_code, command);
    else if (fork() == 0)
        execvp(cmd, command->argv);
    else
    {
        int status;
        waitpid(0, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }
}
