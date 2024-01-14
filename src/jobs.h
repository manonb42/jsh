#ifndef JOBS_H_
#define JOBS_H_

#include "jsh.h"
#include <stdio.h>

void job_track(job_t *job);
void job_untrack(job_t *job);

job_t *job_by_id(int jid);
int job_count();

void job_update_state(job_t *job);
void process_update_state(int pid, int wstatus);

void job_display_state(job_t *job, FILE *output);
void job_notify_state(job_t *job);
void job_notify_state_changes();
void job_update_background_states();

#endif // JOBS_H_
