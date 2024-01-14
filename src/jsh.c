#include "jsh.h"
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <wait.h>
#include <signal.h>

#include "input.h"
#include "exec.h"
#include "jobs.h"

jsh_t jsh = {0};

void ignore_signals(){
    struct sigaction ignore = { .sa_handler = SIG_IGN };
    int sig_to_ignore[] = {SIGINT, SIGQUIT, SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU};

    for (unsigned i = 0; i < sizeof(sig_to_ignore) / sizeof(int); ++i)
        sigaction(sig_to_ignore[i], &ignore, NULL);
}

void default_signals(){
    // Initializing signals handler
    struct sigaction deflt = { .sa_handler = SIG_DFL };
    int sig_to_default[] = {SIGINT, SIGQUIT, SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU};

    //  Default action
    for (int i = 0; i < sizeof(sig_to_default) / sizeof(int); ++i)
        sigaction(sig_to_default[i], &deflt, NULL);

}
void free_command(command_t *command)
{
    for (int i = 0; command->argv[i]; ++i)
        free(command->argv[i]);
    free(command->stdin.path);
    free(command->stdout.path);
    free(command->stderr.path);
    free(command->argv);
    free(command->line);
    free(command);
}

void free_pipeline(pipeline_t *pipeline)
{
    for (int i=0; i<vector_length(&pipeline->commands); ++i)
        free_command(vector_at(&pipeline->commands, i));
    vector_free(pipeline->commands);
    free(pipeline->line);
    free(pipeline);
}

void free_process(process_t *process) {
    free(process->line);
    free(process);
}

void free_job(job_t *job)
{
    for (int i=0; i<vector_length(&job->processes); ++i)
        free_process(vector_at(&job->processes, i));
    vector_free(job->processes);
    free(job->line);
    free(job);
}


int main()
{
    rl_initialize();
    rl_outstream = stderr;

    ignore_signals();

    while (1)
    {
        pipeline_t *pipeline;
        do
        {
            // Update status of all processes of each job
            job_update_background_states();
            job_notify_state_changes();
            pipeline = read_pipeline();
        } while (pipeline == NULL);
        exec_command(vector_at(&pipeline->commands, 0));
        free_pipeline(pipeline);
    }

    return 0;
}
