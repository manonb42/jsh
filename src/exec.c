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
	command->nb_jobs++;

	if (is_internal(cmd))
	{
		command->bg = false;
		jsh.last_exit_code = exec_internal(command);
	}
	else
		exec_external(command);
}

void exec_external(command_t *command)
{
	int pid_bg = 0;
	int pid;
	char *cmd = command->argv[0];

	if (command->bg)
	{
		command->nb_jobs++;
		if ((pid_bg = fork()) != 0)
			command->bg = false;
	}

	jsh.pid_to_stop = pid_bg;

	if (pid_bg == 0 && (pid = fork()) == 0)
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
