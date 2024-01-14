#include "internalcmd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>

#include "jobs.h"
#include "exec.h"

char *internals[] = {"pwd", "cd", "exit", "?", "kill", "jobs", "bg", "fg"};

void dperror(int fd, char *prefix){
    dprintf(fd, "%s : %s\n", prefix, strerror(errno));
}

bool parse_number(char *s, int *out)
{
    char *end;
    int code = strtol(s, &end, 10);

    if (*end != '\0')
        return 0;
    *out = code;
    return 1;
}

int exec_pwd(command_t *command)
{
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)))
    {
        dprintf(command->stdout.fd, "%s\n", curdir);
        return 0;
    }
    else
    {
        dperror(command->stderr.fd, "jsh: pwd");
        return 1;
    }
}

int exec_cd(command_t *command)
{
    char *path = command->argv[1];

    char curdir[1024]; // pwd of current directory
    getcwd(curdir, sizeof(curdir));
    char newpath[1024]; // The path's directory where we want to go.

    if (!path)
        strcpy(newpath, getenv("HOME"));
    else if (*path == '~')
    {
        // Case : Invalid use of "~" path
        if (*(path + 1) != '/' && *(path + 1) != '\0')
        {
            dprintf(command->stderr.fd, "jsh: cd: invalid path's syntax ('%s')\n", path);
            return 1;
        }
        sprintf(newpath, "%s%s", getenv("HOME"), path + 1);
    }
    else if (!strcmp(path, "-"))
    {
        // Case : No previous directory
        if (!getenv("OLDPWD"))
        {
            dprintf(command->stderr.fd, "jsh: cd: \"OLDPWD\" is not define (no previous directory)\n");
            return 1;
        }
        strcpy(newpath, getenv("OLDPWD"));
    }
    else
        strcpy(newpath, path);

    if (chdir(newpath))
    {
        dperror(command->stderr.fd, "jsh: cd");
        return 1;
    }
    // Change previous directory
    setenv("OLDPWD", curdir, 1);
    return 0;
}

int exec_exit( command_t *command)
{
    int code = jsh.last_exit_code;
    if (job_count() > 0)
    {
        dprintf(command->stderr.fd, "jsh: exit: there are running or stopped jobs\n");
        return 1;
    }

    if (command->argc > 1 && !parse_number(command->argv[1], &code))
    {
        dprintf(command->stderr.fd, "jsh: exit: bad argument\n");
        return 1;
    }

    exit(code);
    return 0;
}

int exec_show_last_return_code(command_t *command)
{
    if (dprintf(command->stdout.fd, "%d\n", jsh.last_exit_code) < 0)
    {
        dperror(command->stderr.fd, "jsh: ?");
        return 1;
    }
    return 0;
}

int exec_jobs(command_t *command)
{
    if(command->argc > 2) {
        dprintf(command->stderr.fd, "jsh: bg: bad argument\n");
        return 1;
    }
    int job_to_display = 0; //if we display all jobs
    if(command->argc == 2 && !strcmp((command->argv)[1], "-t")) {
    //jobs -t
    }
    if(command->argc == 2 && ((command->argv)[1][0] == '%')) {
    //jobs %job - assuming number of jobs < 10
        job_t *job = job_by_id(atoi((command->argv)[1] + 1));
        if(job == NULL){
            dprintf(command->stderr.fd, "jsh: bg: bad argument\n");
            return 1;
        }
        job_to_display = job->jid;
    }
    job_update_background_states();

    for (int i = 0; i < vector_length(&jsh.jobs); ++i)
    {
        job_t *proc = vector_at(&jsh.jobs, i);
        if (!proc)
            continue;
        if (!job_to_display || (job_to_display == proc->jid))
            job_notify_state(proc);
    }
    return 0;
}

int exec_bg(command_t *command)
{
    //assuming number of jobs < 10
    if(command->argc != 2 || (command->argv)[1][0] != '%'){
        dprintf(command->stderr.fd, "jsh: bg: bad argument\n");
        return 1;
    }
    job_t *job_to_bg = job_by_id(atoi((command->argv)[1] + 1));
    if(job_to_bg == NULL){
        dprintf(command->stderr.fd, "jsh: bg: bad argument\n");
        return 1;
    }
    kill(job_to_bg->pgid, SIGCONT);
    return 0;
}

int exec_fg(command_t *command)
{
    //assuming number of jobs < 10
    if(command->argc != 2 || (command->argv)[1][0] != '%'){
        dprintf(command->stderr.fd, "jsh: fg: bad argument\n");
        return 1;
    }

    job_t *job_to_fg = job_by_id(atoi((command->argv)[1] + 1));
    if(job_to_fg == NULL) {
        dprintf(command->stderr.fd, "jsh: fg: bad argument\n");
        return 1;
    }
    job_to_fg->running_fg = 1;
    put_process_in_foreground(job_to_fg->pgid);
    kill(job_to_fg->pgid, SIGCONT);
    int status;
    sleep(1);
    int pid = waitpid(-job_to_fg->pgid, &status, WUNTRACED | WCONTINUED);
    put_process_in_foreground(getpgrp());
    process_update_state(pid, status);
    if (job_to_fg->current_state >= P_DONE)
        job_to_fg->notified_state = job_to_fg->current_state;
    if (job_to_fg->current_state == P_DONE)
        return WEXITSTATUS(status);
    job_display_state(job_to_fg,stderr);
    return 0;
}

int exec_kill(command_t *command)
{
    if (command->argc < 2 || command->argc > 3)
        dprintf(command->stderr.fd, "jsh: kill: bad argument\n");

    char *sig = command->argc == 3 ? command->argv[1] : "-15";
    char *target = command->argv[command->argc - 1];

    bool job = target[0] == '%';
    int signum = 0;
    int pid = 0;

    if (sig[0] != '-' || !parse_number(&sig[1], &signum))
    {
        dprintf(command->stderr.fd, "jsh: kill: bad argument\n");
        return 1;
    }

    int targetnum = 0;
    if (!parse_number(&target[job ? 1 : 0], &targetnum))
    {
        dprintf(command->stderr.fd, "jsh: kill: bad argument\n");
        return 1;
    }

    if (job)
    {
        job_t *job = job_by_id(targetnum);
        if (!job)
        {
            dprintf(command->stderr.fd, "jsh: kill: bad job id\n");
            return 1;
        }
        pid = -job->pgid;
    }
    else
        pid = targetnum;

    kill(pid, signum);
    return 0;
}

bool is_internal(char *name)
{
    for (unsigned long i = 0; i < sizeof(internals) / sizeof(char *); ++i)
        if (!strcmp(name, internals[i]))
            return true;
    return false;
}

void exec_internal(command_t *command, job_t *job){
    char *cmd = command->argv[0];
    int code;
    if (!strcmp(cmd, "?")) code = exec_show_last_return_code(command);
    else if (!strcmp(cmd, "pwd")) code = exec_pwd(command);
    else if (!strcmp(cmd, "cd")) code = exec_cd(command);
    else if (!strcmp(cmd, "exit")) code = exec_exit(command);
    else if (!strcmp(cmd, "jobs")) code = exec_jobs(command);
    else if (!strcmp(cmd, "kill")) code = exec_kill(command);
    else if (!strcmp(cmd, "bg")) code = exec_bg(command);
    else if (!strcmp(cmd, "fg")) code = exec_fg(command);
    else code = -1;
    register_process(job, command, 0, P_DONE, code);
}
