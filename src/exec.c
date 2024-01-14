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
#include <signal.h>

#include "internalcmd.h"
#include "jobs.h"

int setup_redir_fd(command_redir_t *redir, int default_fd){
  int fd;
  switch (redir->type) {
    case R_NONE: fd = default_fd; break;
    case R_INPUT: fd = open(redir->path, O_RDONLY | O_CLOEXEC); break;
    case R_NO_CLOBBER: fd = open(redir->path, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_CLOEXEC, 0664); break;
    case R_CLOBBER: fd = open(redir->path, O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC, 0664); break;
    case R_APPEND: fd = open(redir->path, O_CREAT | O_WRONLY | O_APPEND | O_CLOEXEC, 0664); break;
  }
  if (fd >= 0) { redir->fd = fd; return 0;}
  else { perror("jsh"); return -1; }
}

void put_process_in_foreground(pid_t pid_grp)
{
    // Initializing the mask
    sigset_t s_courant, s_bloque;
    sigemptyset(&s_bloque);
    sigaddset(&s_bloque, SIGTTOU);

    // Blocking SIGTTOU
    sigprocmask(SIG_SETMASK, &s_bloque, &s_courant);

    // Critical section
    tcsetpgrp(STDIN_FILENO, pid_grp);

    // Unblocking SIGTTOU
    sigprocmask(SIG_SETMASK, &s_courant, NULL);
}

int exec_external(command_t *command)
{
    int pid = fork();

    if (pid < 0)
    {
        perror("jsh");
        exit(2);
    }

    if (!pid)
    {
        // Process group created for pid
        setpgid(0, 0);
        // Put this process group in foreground
        if (!command->bg)
            put_process_in_foreground(getpgrp());

        execvp(command->argv[0], command->argv);
        perror("jsh");
        exit(127);
    }

    // Shell process group
    setpgid(pid, 0);
    job_t *job = calloc(sizeof(job_t), 1);
    *job = (job_t){.pgid = pid, .current_state = P_RUNNING, .notified_state = P_NONE, .line = strdup(command->line)};
    job_track(job);

    // If cmd is in background
    if (!command->bg)
    {
        // Blocking until the command is completed
        int status;
        waitpid(-pid, &status, WUNTRACED | WCONTINUED);
        // Put the process group of the shell in foreground
        put_process_in_foreground(getpgrp());
        // Update status of all processes in the job
        job_update_state(job, status);
        if (job->current_state >= P_DONE)
            job->notified_state = job->current_state;
        if (job->current_state == P_DONE)
            return WEXITSTATUS(status);
    }
    return 0;
}

void exec_command(command_t *command)
{

    if (setup_redir_fd(&command->stdin, STDIN_FILENO) != 0){
        jsh.last_exit_code = 1; return; }
    if (setup_redir_fd(&command->stdout, STDOUT_FILENO) != 0){
        jsh.last_exit_code = 1; return; }
    if (setup_redir_fd(&command->stderr, STDERR_FILENO) != 0){
        jsh.last_exit_code = 1; return; }

    char *cmd = command->argv[0];

    if (is_internal(cmd))
    {
        command->bg = false;
        jsh.last_exit_code = exec_internal(command);
        return;
    }


    int original_stdin = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);
    int original_stderr = dup(STDERR_FILENO);

    if (command->stdin.fd != STDIN_FILENO){
        dup2(command->stdin.fd, STDIN_FILENO);
        close(command->stdin.fd);
    }
    if (command->stdout.fd != STDOUT_FILENO){
        dup2(command->stdout.fd, STDOUT_FILENO);
        close(command->stdout.fd);
    }
    if (command->stderr.fd != STDERR_FILENO){
        dup2(command->stderr.fd, STDERR_FILENO);
        close(command->stderr.fd);
    }

    // Initializing signals handler
    struct sigaction ignore = {0}, def = {0};
    ignore.sa_handler = SIG_IGN;
    def.sa_handler = SIG_DFL;
    int sig_to_ignore[] = {SIGINT, SIGQUIT, SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU};

    //  Default action
    for (int i = 0; i < sizeof(sig_to_ignore) / sizeof(int); ++i)
        sigaction(sig_to_ignore[i], &def, NULL);

    jsh.last_exit_code = exec_external(command);

    // Ignore signals
    for (int i = 0; i < sizeof(sig_to_ignore) / sizeof(int); ++i)
        sigaction(sig_to_ignore[i], &ignore, NULL);

    // Default input/output
    dup2(original_stdin, STDIN_FILENO);
    dup2(original_stdout, STDOUT_FILENO);
    dup2(original_stderr, STDERR_FILENO);

    close(original_stdin);
    close(original_stdout);
    close(original_stderr);
}
