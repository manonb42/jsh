#include "internalcmd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

#include "jobs.h"
#include "exec.h"

char *internals[] = {"pwd", "cd", "exit", "?", "kill", "jobs", "bg", "fg"};

bool parse_number(char *s, int *out)
{
    char *end;
    int code = strtol(s, &end, 10);

    if (*end != '\0')
        return 0;
    *out = code;
    return 1;
}

int exec_pwd()
{
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)))
    {
        printf("%s\n", curdir);
        return 0;
    }
    else
    {
        perror("jsh: pwd");
        return 1;
    }
}

int exec_cd(char *path)
{
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
            fprintf(stderr, "jsh: cd: invalid path's syntax ('%s')\n", path);
            return 1;
        }
        sprintf(newpath, "%s%s", getenv("HOME"), path + 1);
    }
    else if (!strcmp(path, "-"))
    {
        // Case : No previous directory
        if (!getenv("OLDPWD"))
        {
            fprintf(stderr, "jsh: cd: \"OLDPWD\" is not define (no previous directory)\n");
            return 1;
        }
        strcpy(newpath, getenv("OLDPWD"));
    }
    else
        strcpy(newpath, path);

    if (chdir(newpath))
    {
        perror("jsh: cd");
        return 1;
    }
    // Change previous directory
    setenv("OLDPWD", curdir, 1);
    return 0;
}

int exec_exit(int code, command_t *command)
{
    if (job_count() > 0)
    {
        fprintf(stderr, "jsh: exit: there are running or stopped jobs\n");
        return 1;
    }

    if (command->argc > 1 && !parse_number(command->argv[1], &code))
    {
        fprintf(stderr, "jsh: exit: bad argument\n");
        return 1;
    }

    exit(code);
    return 0;
}

int exec_show_last_return_code()
{
    if (printf("%d\n", jsh.last_exit_code) < 0)
    {
        perror("jsh: ?");
        return 1;
    }
    return 0;
}

int exec_jobs()
{
    job_update_background_states();

    for (int i = 0; i < vector_length(&jsh.jobs); ++i)
    {
        job_t *proc = vector_at(&jsh.jobs, i);
        if (!proc)
            continue;

        job_notify_state(proc);
    }
    return 0;
}

int exec_bg(command_t *command)
{
    if(command->argc != 2 || (command->argv)[1][0] != '%'){
        fprintf(stderr, "jsh: bg: bad argument\n");
        return 1;
    }
    job_t *job_to_bg = job_by_id(atoi((command->argv)[1] + 1));
    if(job_to_bg == NULL){
        fprintf(stderr, "jsh: bg: bad argument\n");
        return 1;
    }
    kill(job_to_bg->pgid, SIGCONT);
    return 0;
}

int exec_fg(command_t *command)
{
    if(command->argc != 2 || (command->argv)[1][0] != '%'){
        fprintf(stderr, "jsh: fg: bad argument\n");
        return 1;
    }

    job_t *job_to_fg = job_by_id(atoi((command->argv)[1] + 1));
    if(job_to_fg == NULL) {
        fprintf(stderr, "jsh: fg: bad argument\n");
        return 1;
    }
    job_to_fg->running_fg = 1;
    put_process_in_foreground(job_to_fg->pgid);
    kill(job_to_fg->pgid, SIGCONT);
    int pid = job_to_fg->pgid;
    int status;
    sleep(1);
    waitpid(-pid, &status, WUNTRACED | WCONTINUED);
    put_process_in_foreground(getpgrp());
    job_update_state(job_to_fg, status);
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
        fprintf(stderr, "jsh: kill: bad argument\n");

    char *sig = command->argc == 3 ? command->argv[1] : "-15";
    char *target = command->argv[command->argc - 1];

    bool job = target[0] == '%';
    int signum = 0;
    int pid = 0;

    if (sig[0] != '-' || !parse_number(&sig[1], &signum))
    {
        fprintf(stderr, "jsh: kill: bad argument\n");
        return 1;
    }

    int targetnum = 0;
    if (!parse_number(&target[job ? 1 : 0], &targetnum))
    {
        fprintf(stderr, "jsh: kill: bad argument\n");
        return 1;
    }

    if (job)
    {
        job_t *job = job_by_id(targetnum);
        if (!job)
        {
            fprintf(stderr, "jsh: kill: bad job id\n");
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

int exec_internal(command_t *command)
{
    char *cmd = command->argv[0];
    if (!strcmp(cmd, "?"))
        return exec_show_last_return_code();
    else if (!strcmp(cmd, "pwd"))
        return exec_pwd();
    else if (!strcmp(cmd, "cd"))
        return exec_cd(command->argv[1]);
    else if (!strcmp(cmd, "exit"))
        return exec_exit(jsh.last_exit_code, command);
    else if (!strcmp(cmd, "jobs"))
        return exec_jobs(command);
    else if (!strcmp(cmd, "kill"))
        return exec_kill(command);
    else if (!strcmp(cmd, "bg"))
        return exec_bg(command);
    else if (!strcmp(cmd, "fg"))
        return exec_fg(command);
    return -1;
}
