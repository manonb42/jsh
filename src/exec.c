#include "exec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

void exec_command(command_t *command){

    if (strcmp (command->argv[0],"?") == 0){
        printf("%d\n", jsh.last_exit_code);
    }
    else if (strcmp (command->argv[0],"pwd") == 0){
        char buf[1024];
        if(getcwd(buf, sizeof(buf)) != NULL) {
            printf("%s\n", buf);
        }
        printf("pwd Failure\n");
    }
    else if (fork() == 0) {
        execvp(command->argv[0], command->argv);
    } else {
        int status;
        waitpid(0, &status, 0);
        jsh.last_exit_code = WEXITSTATUS(status);
    }

}
