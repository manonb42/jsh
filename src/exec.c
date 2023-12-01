#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

#include "internalcmd.h"

void exec_command(command_t *command)
{
	int pid_bg = 0;
	int pid;
	int status_bg;

    char *cmd = command->argv[0];

	command->nb_jobs++;
	if(command->bg) {
		command->nb_jobs++;
		if((pid_bg = fork()) != 0) {
			command->bg = false;
		}
	}
	if (waitpid(pid_bg, &status_bg, WNOHANG) > 0 ){
		command->nb_jobs--;
	}
    if (strcmp(cmd, "?") == 0)
        jsh.last_exit_code = showLastReturnCode();
    else if (strcmp(cmd, "pwd") == 0)
        jsh.last_exit_code = pwd();
    else if (strcmp(cmd, "cd") == 0)
        jsh.last_exit_code = cd(command->argv[1]);
    else if (strcmp(cmd, "exit") == 0)
        jsh.last_exit_code = quit(jsh.last_exit_code, command);
    else if (pid_bg == 0 && (pid = fork()) == 0)
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
		waitpid(pid, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }
}
