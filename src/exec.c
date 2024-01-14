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

void register_process(job_t *job, command_t *command, int pid, process_state_t state, int exit){
    process_t *process = calloc(1, sizeof(process_t));
    *process = (process_t){ .pid = pid,
                            .state = state,
                            .exit_code = exit,
                            .line = strdup(command->line)};
    vector_append(&job->processes, process);
}

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

void job_foreground(job_t *job){
    job_update_state(job);

    if (job->current_state == P_DONE){
         process_t *process = vector_at(&job->processes, 0);
         job->notified_state = job->current_state;
         jsh.last_exit_code = process->exit_code;
         return;
    }

     put_process_in_foreground(job->pgid);
     int status;
     while (job->current_state == P_RUNNING){
        int pid = waitpid(-job->pgid, &status, WUNTRACED | WCONTINUED);
        process_update_state(pid, status);
     }

    if (job->current_state >= P_DONE)
        job->notified_state = job->current_state;
    if (job->current_state == P_DONE){
        job->notified_state = job->current_state;
        process_t *process = vector_at(&job->processes, vector_length(&job->processes)-1);
        jsh.last_exit_code = process->exit_code;
    }

    put_process_in_foreground(getpgrp());
}

void exec_external_init_child(command_t *command, job_t *job){
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

    if (pid < 0)
    {
        perror("jsh");
        exit(2);
    }

    if (!pid){ exec_external_init_child(command, job);}

    if (job->pgid == 0) job->pgid = pid;
    setpgid(pid, job->pgid);

    register_process(job, command, pid, P_RUNNING, 0);

}

void exec_command(command_t *command, job_t *job){

    if (setup_redir_fd(&command->stdin, STDIN_FILENO) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }
    if (setup_redir_fd(&command->stdout, STDOUT_FILENO) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }
    if (setup_redir_fd(&command->stderr, STDERR_FILENO) != 0){
        register_process(job, command, 0, P_DONE, 1); return; }

    char *cmd = command->argv[0];

    if (is_internal(cmd)) exec_internal(command, job);
    else  exec_external(command, job);


    if (command->stdin.fd  != STDIN_FILENO)  close(command->stdin.fd);
    if (command->stdout.fd != STDOUT_FILENO) close(command->stdout.fd);
    if (command->stderr.fd != STDERR_FILENO) close(command->stderr.fd);
}

void exec_pipeline(pipeline_t *pipeline){
    job_t *job = calloc(1, sizeof(job_t));
    *job = (job_t){
        .pgid = 0,
        .current_state = P_RUNNING,
        .notified_state = P_NONE,
        .line = strdup(pipeline->line)};

    for (int i=0; i<vector_length(&pipeline->commands); ++i){
        command_t *command = vector_at(&pipeline->commands, i);
        exec_command(command, job);
    }

    job_track(job);
    if (!pipeline->background) job_foreground(job);
}
