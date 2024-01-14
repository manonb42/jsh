#ifndef EXEC_H_
#define EXEC_H_

#include "jsh.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void put_process_in_foreground(pid_t pid_grp);
void exec_pipeline(pipeline_t *);

void register_process(job_t *job, command_t *command, int pid, process_state_t state, int exit);
void job_foreground(job_t *job);
#endif // EXEC_H_
