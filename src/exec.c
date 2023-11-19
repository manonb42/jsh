#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>


void exec_command(command_t *command){

    char *name = command->argv[0];

    if (strcmp (name,"?") == 0){
        printf("%d\n", jsh.last_exit_code);
    }
    else if (strcmp (name,"pwd") == 0){
        char buf[1024];
        if(getcwd(buf, sizeof(buf)) != NULL) {
            printf("%s\n", buf);
        } else {
            printf("pwd failure\n");
        }
    }
    else if (strcmp (name,"exit") == 0){
        int code = jsh.last_exit_code;
        if (command->argc == 2){
            char *end;
            code = strtol(command->argv[1], &end, 10);
            if (*end != '\0') {
                printf("exit: bad argument");
                return;
            }
        }
        exit(code);
    }
    else if (fork() == 0) {
        execvp(name, command->argv);
    } else {
        int status;
        waitpid(0, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }

}
