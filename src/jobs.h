#ifndef JOBS_H_
#define JOBS_H_

#include "jsh.h"

int job_count();
void job_untrack(process_t *proc);
void job_track(process_t *proc);

void job_notify_state(process_t *proc);
void job_notify_state_changes();
void job_update_background_states();


process_t *job_by_id(int jid);
#endif // JOBS_H_
