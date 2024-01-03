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

int main()
{
    rl_initialize();
    rl_outstream = stderr;

    // Initializing signals handler
    struct sigaction ignore = {0};
    ignore.sa_handler = SIG_IGN;
    int sig_to_ignore[] = {SIGINT, SIGQUIT, SIGTERM, SIGTSTP, SIGTTIN, SIGTTOU};

    // Ignore signals
    for (int i = 0; i < sizeof(sig_to_ignore) / sizeof(int); ++i)
        sigaction(sig_to_ignore[i], &ignore, NULL);

    while (1)
    {
        command_t *command;
        do
        {
            // Update status of all processes of each job
            job_update_background_states();
            job_notify_state_changes();
            command = read_command();
        } while (command == NULL);
        exec_command(command);
        free_command(command);
    }

    return 0;
}
