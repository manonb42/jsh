#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "internalcmd.h"

int get_fd(command_t *command) {
	if (!strcmp(command->redir, "<"))
        return open(command->fic, O_RDONLY);
    if (!strcmp(command->redir, ">"))
        return open(command->fic, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (!strcmp(command->redir, ">|"))
        return open(command->fic, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (!strcmp(command->redir, ">>")) {
        return open(command->fic, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
	}
	return -1;
}

int asserting(int n, command_t *command) {
	if	(n < 0) {
		perror("jsh open");
		return -1;
	}
	return 0;
}


void exec_command(command_t *command)
{
	int original_stdout = dup(STDOUT_FILENO);
	int original_stderr = dup(STDERR_FILENO);
	int original_stdin = dup(STDIN_FILENO);

	if(command->is_redir) {
		int fd = get_fd(command);
		if(asserting(fd, command) < 0) {
			return;
		}
		int d = dup2(fd, command->descr);
		if(asserting(d, command) < 0) {
			return;
		}
		close(fd);
	}
	char *cmd = command->argv[0];

	if (is_internal(cmd)) {
		command->bg = false;
		jsh.last_exit_code = exec_internal(command);
	}
	else {
		exec_external(command);
	}
	if(command->is_redir) {
		if(command->descr == STDOUT_FILENO) {
		dup2(original_stdout, STDOUT_FILENO);
		}
		if(command->descr == STDIN_FILENO) {
		dup2(original_stdout, STDIN_FILENO);
		}
		else {

		dup2(original_stderr, STDERR_FILENO);
		}
		close(original_stdout);
		close(original_stderr);
		close(original_stdin);
	}
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
