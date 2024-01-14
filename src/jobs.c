#include "jobs.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

int job_next_id()
{
  for (int i = 0; i < vector_length(&jsh.jobs); ++i)
    if (!vector_at(&jsh.jobs, i))
      return i + 1;
  return vector_length(&jsh.jobs) + 1;
}

void job_track(job_t *job)
{
  job->jid = job_next_id();

  if (job->jid - 1 < vector_length(&jsh.jobs))
    vector_set(&jsh.jobs, job->jid - 1, job);
  else
    vector_append(&jsh.jobs, job);
}

void job_untrack(job_t *job)
{
  vector_set(&jsh.jobs, job->jid - 1, NULL);
  job->jid = 0;
}


job_t *job_by_id(int jid)
{
  if (vector_length(&jsh.jobs) <= jid - 1)
    return NULL;
  return vector_at(&jsh.jobs, jid - 1);
}

int job_count()
{
  int out = 0;
  for (int i = 0; i < vector_length(&jsh.jobs); ++i)
    if (vector_at(&jsh.jobs, i))
      out++;
  return out;
}


void job_display_state(job_t *job, FILE *output)
{
  char *state;
  switch (job->current_state)
  {
  case P_NONE:
    state = "???";
    break;
  case P_RUNNING:
    state = "Running";
    break;
  case P_STOPPED:
    state = "Stopped";
    break;
  case P_DONE:
    state = "Done";
    break;
  case P_KILLED:
    state = "Killed";
    break;
  case P_DETACHED:
    state = "Detached";
    break;
  }
  if(!job->running_fg || ((job->current_state != P_RUNNING) && (job->current_state != P_DONE))) {
    fprintf(output, "[%d] %d\t%s\t%s\n", job->jid, job->pgid, state, job->line);
  }
}

void job_notify_state(job_t *job)
{
  job_display_state(job, stdout);
  job->notified_state = job->current_state;

  if (job->current_state >= P_DONE)
  {
    job_untrack(job);
    free_job(job);
  }
}

void job_notify_state_changes()
{
  for (int i = 0; i < vector_length(&jsh.jobs); ++i)
  {
    job_t *job = vector_at(&jsh.jobs, i);
    if (!job)
      continue;

    // We only display changed states (unlike in `job_notify_state`)
    if (job->current_state != job->notified_state)
      job_display_state(job, stderr);

    job->notified_state = job->current_state;
    if (job->current_state >= P_DONE)
    {
      job_untrack(job);
      free_job(job);
    }
  }
}

void job_update_state(job_t *job, int wstatus)
{
  if (WIFEXITED(wstatus))
    job->current_state = P_DONE;
  if (WIFSIGNALED(wstatus))
    job->current_state = P_KILLED;
  if (WIFSTOPPED(wstatus))
    job->current_state = P_STOPPED;
  if (WIFCONTINUED(wstatus))
    job->current_state = P_RUNNING;
}

void job_update_background_states()
{
  int pid, wstatus;
  while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
  {
    for (int i = 0; i < vector_length(&jsh.jobs); ++i)
    {
      job_t *job = vector_at(&jsh.jobs, i);
      if (!job)
        continue;
      if (job->pgid != pid)
        continue;
      job_update_state(job, wstatus);
    }
  }
}

