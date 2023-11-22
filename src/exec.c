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
        jsh.last_exit_code = pwd();
    else if (strcmp(cmd, "cd") == 0)
        jsh.last_exit_code = cd(command->argv[1]);
    else if (strcmp(cmd, "exit") == 0)
        jsh.last_exit_code = quit(jsh.last_exit_code, command);
    else if (fork() == 0) {
        execvp(cmd, command->argv);
		exit(1);
	}
    else
    {
        int status;
        waitpid(0, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }
}
