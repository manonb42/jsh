#include "internalcmd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "jobs.h"

char *internals[] = {"pwd", "cd", "exit", "?", "kill", "jobs"};


bool parse_number(char *s, int *out){
    char *end;

    int code = strtol(s, &end, 10);
    if (*end != '\0'){
        return 0;
    }
    *out = code;
    return 1;
}

int exec_pwd()
{
    char curdir[1024];
    if (getcwd(curdir, sizeof(curdir)) != NULL)
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

    if (path == NULL)
        strcpy(newpath, getenv("HOME"));
    else if (*path == '~')
    {
        // Case : Invalid use of "~" path
        if (*(path + 1) != '/' && *(path + 1) != '\0')
        {
            fprintf(stderr, "\tcd: invalid path's syntax ('%s')\n", path);
            return 1;
        }
        sprintf(newpath, "%s%s", getenv("HOME"), path + 1);
    }
    else if (strcmp(path, "-") == 0)
    {
        // Case : No previous directory
        if (getenv("OLDPWD") == NULL)
        {
            fprintf(stderr, "jsh: cd: \"OLDPWD\" is not define (no previous directory)\n");
            return 1;
        }
        strcpy(newpath, getenv("OLDPWD"));
    }
    else
        strcpy(newpath, path);

    if (chdir(newpath) != 0)
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
    if (job_count() > 0) return 1;

    if (!parse_number(command->argv[1], &code)) {
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

int exec_jobs(){
    job_update_background_states();

    for (int i=0; i < vector_length(&jsh.processes); ++i){

        process_t* proc = vector_at(&jsh.processes, i);
        if (proc == NULL) continue;

        job_notify_state(proc);
    }
    return 0;
}

int exec_kill(command_t *command){
    if (command->argc < 2) fprintf(stderr, "jsh: kill: missing argument\n");

    char *sig = command->argc == 3 ? command->argv[1] : "-9";
    char *target = command->argv[command->argc-1];

    bool job = target[0] == '%';
    int signum = 0;
    int pid = 0;

    if (sig[0] != '-' || !parse_number(&sig[1], &signum)){
        fprintf(stderr, "jsh: kill: bad argument\n");
        return 1;
    }

    int targetnum = 0;
    if (!parse_number(&target[job ? 1 : 0], &targetnum) ){
        fprintf(stderr, "jsh: kill: bad argument\n");
        return 1;
    }

    if (job) {
        process_t *proc = job_by_id(targetnum);
        if (proc == NULL){ fprintf(stderr, "jsh: kill: bad job id\n"); return 1; }
        pid = proc->pid;
    } else { pid = targetnum; }

    kill(pid, signum);

    return 0;
}

bool is_internal(char *name)
{
    for (unsigned long i = 0; i < sizeof(internals) / sizeof(char *); ++i)
        if (strcmp(name, internals[i]) == 0)
            return true;
    return false;
}

int exec_internal(command_t *command)
{
    char *cmd = command->argv[0];
    if (strcmp(cmd, "?") == 0)
        return exec_show_last_return_code();
    else if (strcmp(cmd, "pwd") == 0)
        return exec_pwd();
    else if (strcmp(cmd, "cd") == 0)
        return exec_cd(command->argv[1]);
    else if (strcmp(cmd, "exit") == 0)
        return exec_exit(jsh.last_exit_code, command);
    else if (strcmp(cmd, "jobs") == 0)
        return exec_jobs(command);
    else if (strcmp(cmd, "kill") == 0)
        return exec_kill(command);
    return -1;
}
