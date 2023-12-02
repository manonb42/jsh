#include "internalcmd.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


char *internals[] = { "pwd", "cd", "exit", "?"};

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
    if (command->argc == 2)
    {
        char *end;
        code = strtol(command->argv[1], &end, 10);
        if (*end != '\0')
        {
            fprintf(stderr, "jsh: exit: bad argument\n");
            return 1;
        }
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

bool is_internal(char *name){
    for (unsigned long i=0; i < sizeof(internals) / sizeof(char*); ++i)
        if (strcmp(name, internals[i]) == 0) return true;
    return false;
}

int exec_internal(command_t *command){
    char *cmd = command->argv[0];
    if (strcmp(cmd, "?") == 0) return exec_show_last_return_code();
    else if (strcmp(cmd, "pwd") == 0) return exec_pwd();
    else if (strcmp(cmd, "cd") == 0) return exec_cd(command->argv[1]);
    else if (strcmp(cmd, "exit") == 0) return exec_exit(jsh.last_exit_code, command);
    return -1;
}
