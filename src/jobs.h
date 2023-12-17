#ifndef JOBS_H_
#define JOBS_H_

#include "jsh.h"

int job_count();
void job_untrack(job_t *job);
void job_track(job_t *job);

void job_notify_state(job_t *job);
void job_notify_state_changes();
void job_update_background_states();


job_t *job_by_id(int jid);
#endif // JOBS_H_
