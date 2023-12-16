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
#include "jobs.h"

int get_fd(command_redir_t redir) {
    int out;
    switch (redir.type){
        case R_INPUT:      out = open(redir.path, O_RDONLY | O_CLOEXEC); break;
        case R_NO_CLOBBER: out = open(redir.path, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644); break;
        case R_CLOBBER:    out = open(redir.path, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0644); break;
        case R_APPEND:     out = open(redir.path, O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC, 0644); break;
        default: return -1;
    }
    if (out >= 0) return out;
    else { perror("jsh"); return -1; }
}


int exec_external(command_t *command)
{

    int pid = fork();

    if (pid == 0) {
        execvp(command->argv[0], command->argv);
        perror("jsh");
        exit(127);
    }
    else if (!command->bg) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    } else {
        process_t *proc = calloc(sizeof(process_t), 1);
        *proc = (process_t){ .pid = pid, .current_state = P_RUNNING, .notified_state = P_NONE, .line = strdup(command->line)};
        job_track(proc);
        return 0;
    }

}

void exec_command(command_t *command)
{

    int original_stdin  = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);
    int original_stderr = dup(STDERR_FILENO);

    if(command->stdin.type != R_NONE) {
        int fd = get_fd(command->stdin) ;
        if (fd < 0) { jsh.last_exit_code = 1; return; }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if(command->stdout.type != R_NONE) {
        int fd = get_fd(command->stdout) ;
        if (fd < 0) { jsh.last_exit_code = 1; return; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if(command->stderr.type != R_NONE) {
        int fd = get_fd(command->stderr) ;
        if (fd < 0) { jsh.last_exit_code = 1; return; }
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    char *cmd = command->argv[0];

    if (is_internal(cmd)) {
        command->bg = false;
        jsh.last_exit_code = exec_internal(command);
    }
    else {
        jsh.last_exit_code = exec_external(command);
    }

    dup2(original_stdin,  STDIN_FILENO);
    dup2(original_stdout, STDOUT_FILENO);
    dup2(original_stderr, STDERR_FILENO);

    close(original_stdin);
    close(original_stdout);
    close(original_stderr);
}
