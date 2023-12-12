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

	if (is_internal(cmd)) {
		command->bg = false;
		jsh.last_exit_code = exec_internal(command);
	}
	else
		exec_external(command);
}

void exec_external(command_t *command)
{

	int pid = fork();

	if (pid == 0) {
		execvp(command->argv[0], command->argv);
		perror("jsh");
		free_command(command);
		jsh.last_exit_code = 127;
		exit(jsh.last_exit_code);
	}
	else if (!command->bg) {
		int status;
		waitpid(pid, &status, 0);
		jsh.last_exit_code = WEXITSTATUS(status);
	} else {
		process_t *proc = calloc(sizeof(process_t), 1);
		*proc = (process_t){ .pid = pid, .current_state = P_RUNNING, .notified_state = P_NONE};
		vector_append(&jsh.processes, proc);
	}
}
