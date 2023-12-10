#include "jsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <wait.h>

#include "input.h"
#include "exec.h"
#include <wait.h>

jsh_t jsh = {0};

void free_command(command_t *command)
{
    // returns true if command is in background
    for (int i = 0; i < command->argc; ++i)
        free(command->argv[i]);
    free(command->argv);
    free(command);
}


void check_background_jobs(){
    int pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0){
        for (int i=0; i<vector_length(&jsh.processes); ++i){
            process_t* proc = vector_at(&jsh.processes, i);
            if (proc->pid == pid){
                vector_remove(&jsh.processes, i);
                free(proc);
                printf("%d\tDone\n", pid);
                break;
            }
        }
    }
}

int main()
{

    rl_initialize();
    rl_outstream = stderr;

    while (1)
    {
        command_t *command;
        do {
            command = read_command();
            check_background_jobs();
        } while( command == NULL );

        exec_command(command);
        free_command(command);
    }

    return 0;
}
