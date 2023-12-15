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
    for (int i = 0; i < command->argc; ++i)
        free(command->argv[i]);
    free(command->argv);
    free(command);
}

void notify_job_state_changes(){

    for (int i=0; i< vector_length(&jsh.processes); ++i){
        process_t* proc = vector_at(&jsh.processes, i);
        if (proc->current_state == proc->notified_state) continue;

        char *state;
        switch (proc->current_state) {
            case P_NONE: state = "???"; break;
            case P_RUNNING: state = "Running"; break;
            case P_STOPPED: state = "Stopped"; break;
            case P_DONE: state = "Done"; break;
            case P_KILLED: state = "Killed"; break;
            case P_DETACHED: state = "Detached"; break;
        }

        printf("%d\t%s\n", proc->pid, state);
        proc->notified_state = proc->current_state;

        if (proc->current_state >= P_DONE){
            vector_remove(&jsh.processes, i--);
            free(proc);
        }
    }
}

void update_background_job_states(){
    int pid, wstatus;
    while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0){
        for (int i=0; i<vector_length(&jsh.processes); ++i){
            process_t* proc = vector_at(&jsh.processes, i);
            if (proc->pid != pid) continue;
            if (WIFEXITED(wstatus)) proc->current_state = P_DONE;
            if (WIFSIGNALED(wstatus)) proc->current_state = P_KILLED;
            if (WIFSTOPPED(wstatus)) proc->current_state = P_STOPPED;
        }
    }
}

int main(){

    rl_initialize();
    rl_outstream = stderr;

    while (1)
    {
        command_t *command;
        do {
            update_background_job_states();
            notify_job_state_changes();
            command = read_command();
        } while( command == NULL );
        exec_command(command);
		
        free_command(command);
    }

    return 0;
}
