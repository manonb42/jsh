#include "jobs.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

int job_next_id(){
  for (int i=0; i<vector_length(&jsh.processes); ++i)
    if (vector_at(&jsh.processes, i) == NULL) return i+1;
  return vector_length(&jsh.processes)+1;
}

int job_count(){
  int out = 0;
  for (int i=0; i<vector_length(&jsh.processes); ++i)
   if (vector_at(&jsh.processes, i) != NULL) out++;
  return out;
}
void job_untrack(process_t *proc){
  vector_set(&jsh.processes, proc->jid-1, NULL);
  proc->jid = 0;
}

void job_track(process_t *proc){
  proc->jid = job_next_id();

  if (proc->jid-1 < vector_length(&jsh.processes))
    vector_set(&jsh.processes, proc->jid-1, proc);
  else vector_append(&jsh.processes, proc);
}

void job_notify_state_changes(){

    for (int i=0; i < vector_length(&jsh.processes); ++i){

        process_t* proc = vector_at(&jsh.processes, i);
        if (proc == NULL) continue;
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

        fprintf(stderr, "[%d] %d\t%s\t%s\n", proc->jid, proc->pid, state, proc->line);
        proc->notified_state = proc->current_state;

        if (proc->current_state >= P_DONE){
            job_untrack(proc);
            free(proc);
        }
    }
}

void job_update_background_states(){
    int pid, wstatus;
    while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED)) > 0){
        for (int i=0; i<vector_length(&jsh.processes); ++i){
            process_t* proc = vector_at(&jsh.processes, i);
            if (proc == NULL) continue;
            if (proc->pid != pid) continue;
            if (WIFEXITED(wstatus)) proc->current_state = P_DONE;
            if (WIFSIGNALED(wstatus)) proc->current_state = P_KILLED;
            if (WIFSTOPPED(wstatus)) proc->current_state = P_STOPPED;
        }
    }
}
