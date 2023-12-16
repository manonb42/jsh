#include "jobs.h"
#include <stddef.h>
#include <stdio.h>

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
