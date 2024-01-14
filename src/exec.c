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

void fd_set_cloexec(int fd)
{
    int flags = fcntl(fd, F_GETFD);
    flags |= FD_CLOEXEC;
    fcntl(fd, F_SETFD, flags);
}

void fd_clear_cloexec(int fd){
    int flags = fcntl(fd, F_GETFD);
    flags &= ~FD_CLOEXEC;
    fcntl(fd, F_SETFD, flags);
}


void register_process(job_t *job, command_t *command, int pid, process_state_t state, int exit){
    process_t *process = calloc(1, sizeof(process_t));
    *process = (process_t){.pid = pid,
                           .state = state,
                           .exit_code = exit,
                           .line = strdup(command->line)};
    vector_append(&job->processes, process);
}

int setup_redir_fd(command_redir_t *redir){
  int fd;
  switch (redir->type) {
    case R_NONE: fd = redir->fd; break;
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

void job_foreground(job_t *job)
{

    if (job->current_state == P_DONE)
    {
        process_t *process = vector_at(&job->processes, 0);
        job->notified_state = job->current_state;
        jsh.last_exit_code = process->exit_code;
        return;
    }

    put_process_in_foreground(job->pgid);
    int status;
    while (job->current_state == P_RUNNING)
    {
        int pid = waitpid(-job->pgid, &status, WUNTRACED | WCONTINUED);
        process_update_state(pid, status);
    }

    if (job->current_state >= P_DONE)
        job->notified_state = job->current_state;
    if (job->current_state == P_DONE)
    {
        job->notified_state = job->current_state;
        process_t *process = vector_at(&job->processes, vector_length(&job->processes) - 1);
        jsh.last_exit_code = process->exit_code;
    }

    put_process_in_foreground(getpgrp());
}

void exec_external_init_child(command_t *command, job_t *job)
{
    default_signals();

    dup2(command->stdin.fd, STDIN_FILENO);
    dup2(command->stdout.fd, STDOUT_FILENO);
    dup2(command->stderr.fd, STDERR_FILENO);

    setpgid(0, job->pgid);

    execvp(command->argv[0], command->argv);
    perror("jsh");
    exit(127);
}

void exec_external(command_t *command, job_t *job)
{
    int pid = fork();

    if (pid < 0) {
        perror("jsh");
        exit(2);
    }

    if (!pid)
    {
        exec_external_init_child(command, job);
    }

    if (job->pgid == 0)
        job->pgid = pid;
    setpgid(pid, job->pgid);

    register_process(job, command, pid, P_RUNNING, 0);
}

void mkfifotmp(char **path){
    char dir[] = "/tmp/jsh.XXXXXX";
    mkdtemp(dir);
    asprintf(path, "%s/pipe", dir);
    mkfifo(*path, 0600);
}



void exec_command(command_t *command, job_t *job){
    char *cmd = command->argv[0];

    if (setup_redir_fd(&command->stdin) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }
    if (setup_redir_fd(&command->stdout) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }
    if (setup_redir_fd(&command->stderr) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }

    for (int i=0; i<vector_length(&command->substitutions); ++i){
        substitution_t *subst = vector_at(&command->substitutions, i);
        mkfifotmp(&subst->file);

        int fds[2];
        pipe(fds);

        subst->fd = fds[0];
        command_v cmds = subst->pipeline->commands;
        command_t *last = vector_at(&cmds, vector_length(&cmds)-1);
        last->stdout.fd = fds[1];

        fd_set_cloexec(subst->fd);
        fd_set_cloexec(last->stdout.fd);
        exec_pipeline(subst->pipeline);

        asprintf(&command->argv[subst->offset], "/proc/self/fd/%d", fds[0]);

        fd_clear_cloexec(subst->fd);
    }

    if (is_internal(cmd)) exec_internal(command, job);
    else  exec_external(command, job);

    for (int i=0; i<vector_length(&command->substitutions); ++i){
        substitution_t *subst = vector_at(&command->substitutions, i);
        close(subst->fd);
    }

    if (command->stdin.fd  != STDIN_FILENO)  close(command->stdin.fd);
    if (command->stdout.fd != STDOUT_FILENO) close(command->stdout.fd);
    if (command->stderr.fd != STDERR_FILENO) close(command->stderr.fd);
}

void exec_pipeline(pipeline_t *pipeline)
{
    job_t *job = calloc(1, sizeof(job_t));
    *job = (job_t){
        .pgid = 0,
        .current_state = P_RUNNING,
        .notified_state = P_NONE,
        .line = strdup(pipeline->line)};

    int pipe_fds[2];
    pipe_fds[0] = pipe_fds[1] = -1;
    for (int i = 0; i < vector_length(&pipeline->commands); ++i)
    {

        command_t *command = vector_at(&pipeline->commands, i);

        if (pipe_fds[0] != -1) command->stdin.fd = pipe_fds[0];

        if (i + 1 < vector_length(&pipeline->commands))
        {
            pipe(pipe_fds);
            fd_set_cloexec(pipe_fds[0]);
            fd_set_cloexec(pipe_fds[1]);
            command->stdout.fd = pipe_fds[1];
        }

        exec_command(command, job);

    }

    job_update_state(job);
    job_track(job);
    if (!pipeline->background)
        job_foreground(job);
}
