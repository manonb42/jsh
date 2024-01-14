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
  char *state = get_state(job->current_state);
  if (!job->running_fg || ((job->current_state != P_RUNNING) && (job->current_state != P_DONE)))
  {
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

bool pgrp_alive(int pgid)
{
  return kill(-pgid, 0) == 0;
}

void job_update_state(job_t *job)
{
  bool running = false;
  bool stopped = false;
  bool killed = false;
  bool done = true;
  bool completed = true;

  for (int i = 0; i < vector_length(&job->processes); ++i)
  {
    process_t *process = vector_at(&job->processes, i);
    completed &= process->state >= P_DONE;
    done &= process->state == P_DONE;
    killed |= process->state == P_KILLED;
    stopped |= process->state == P_STOPPED;
    running |= process->state == P_RUNNING;
  }
  if (completed && job->pgid != 0 && pgrp_alive(job->pgid))
    job->current_state = P_DETACHED;
  if (done)
    job->current_state = P_DONE;
  if (completed && killed)
    job->current_state = P_KILLED;
  if (!running && stopped)
    job->current_state = P_STOPPED;
  if (running)
    job->current_state = P_RUNNING;
}

int process_find_by_pid(int pid, job_t **job, process_t **process)
{
  for (int i = 0; i < vector_length(&jsh.jobs); ++i)
  {
    job_t *j = vector_at(&jsh.jobs, i);
    if (j == NULL)
      continue;
    for (int i = 0; i < vector_length(&j->processes); ++i)
    {
      process_t *p = vector_at(&j->processes, i);
      if (p->pid != pid)
        continue;
      *process = p;
      *job = j;
      return 0;
    }
  }
  return 1;
}

void process_update_state(int pid, int wstatus)
{
  job_t *job;
  process_t *process;

  if (process_find_by_pid(pid, &job, &process))
  {
    printf("pid %d not found among %d jobs", pid, job_count());
    exit(1);
  }

  if (WIFEXITED(wstatus))
  {
    process->state = P_DONE;
    process->exit_code = WEXITSTATUS(wstatus);
  }
  if (WIFSIGNALED(wstatus))
  {
    process->state = P_KILLED;
    process->exit_code = 128 + WTERMSIG(wstatus);
  }
  if (WIFSTOPPED(wstatus))
    process->state = P_STOPPED;
  if (WIFCONTINUED(wstatus))
    process->state = P_RUNNING;

  job_update_state(job);
}

void job_update_background_states()
{
  int pid, wstatus;
  while ((pid = waitpid(-1, &wstatus, WNOHANG | WUNTRACED | WCONTINUED)) > 0)
  {
    process_update_state(pid, wstatus);
  }
}
