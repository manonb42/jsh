#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#include "internalcmd.h"

int exec_external(command_t *command){
    char *cmd = command->argv[0];
    if (fork() == 0) {
        execvp(cmd, command->argv);
        perror("jsh");
        free_command(command);
        jsh.last_exit_code = 127;
        exit(jsh.last_exit_code);
    }
    else {
        int status;
        waitpid(0, &status, 0);
        return WEXITSTATUS(status);
    }
}

void exec_command(command_t *command)
{

    char *cmd = command->argv[0];
    int exit_code;
    if (is_internal(cmd)) exit_code = exec_internal(command);
    else exit_code = exec_external(command);
    jsh.last_exit_code = exit_code;
}
