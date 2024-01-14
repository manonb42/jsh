#include "input.h"
#include "vector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "jobs.h"

// Global Variables - Colors
char *vert = "\001\033[32m\002";
char *cyan = "\001\033[36m\002";
char *jaune = "\001\033[33m\002";
char *bleu = "\001\033[34m\002";
char *rouge = "\001\033[91m\002";
char *normal = "\001\033[00m\002";

char **split_string(char *chemin, char *separateur)
{
    char *chem = strdup(chemin);
    int size = 1;
    for (int i = 0; i < strlen(chem); ++i)
        if (chem[i] == separateur[0])
            ++size;
    char **composantes = calloc(size + 2, sizeof(char *));
    int i = 0;
    for (char *mot = strtok(chem, separateur); mot != NULL; mot = strtok(NULL, separateur))
    {
        composantes[i] = strdup(mot);
        i++;
    }
    free(chem);
    composantes[size + 1] = NULL;
    return composantes;
}

char *join_strings(int count, char **strs, char *join)
{
    if (count == 0)
        return calloc(1, 1);

    int out_len = 0;
    for (int i = 0; i < count; ++i)
        out_len += strlen(strs[i]);
    out_len += strlen(join) * (count - 1);

    char *out = calloc(1, out_len + 1);
    char *cur = out;
    for (int i = 0; i < count - 1; ++i)
    {
        cur = stpcpy(cur, strs[i]);
        cur = stpcpy(cur, join);
    }
    cur = stpcpy(cur, strs[count - 1]);
    return out;
}

int nb_chiffres(int nb)
{
    int i = 1;
    while ((nb = nb / 10) > 0)
        ++i;
    return i;
}

pipeline_t *parse_pipeline(int partc, char **parts);

command_t *parse_command(int partc, char **parts)
{
    command_t *out = calloc(1, sizeof(command_t));
    vector argv = vector_empty();
    substitution_v substs = vector_empty();

    char *symbols[] = {"<", ">", ">|", ">>", "2>", "2>|", "2>>"};
    command_redir_type_t flags[] = {R_INPUT, R_NO_CLOBBER, R_CLOBBER, R_APPEND, R_NO_CLOBBER, R_CLOBBER, R_APPEND};
    command_redir_t *target[] = {&out->stdin, &out->stdout, &out->stdout, &out->stdout, &out->stderr, &out->stderr, &out->stderr};
    out->stdin.fd = 0;
    out->stdout.fd = 1;
    out->stderr.fd = 2;

    for (int i = 0; i < partc; ++i)
    {
        char *arg = parts[i];
        char *next_arg = parts[i + 1];

        // Managing substitutions
        if (strcmp(arg, "<(") == 0){
            int open = i;
            int close;
            for (close = open+1; close<partc && strcmp(parts[close], ")")!=0; close++);
            if (close == partc){
                fprintf(stderr, "jsh: Syntax error : unclosed '<('\n");
                jsh.last_exit_code = 1;
                return NULL;
            }
            pipeline_t *pipeline = parse_pipeline(close-open-1, &parts[open+1]);
            if (pipeline == NULL) return NULL;
            substitution_t *subst = calloc(1, sizeof(substitution_t));
            vector_append(&argv, "SUBST");
            *subst = (substitution_t){ .pipeline = pipeline, .offset = vector_length(&argv)-1};
            vector_append(&substs, subst);
            i = close;
            continue;
        }
        // Managing redirections
        bool redir = false;
        for (int j = 0; j < sizeof(symbols) / sizeof(char *); ++j)
        {
            if (!strcmp(arg, symbols[j]))
            {
                if (!next_arg || !strcmp(next_arg, "|"))
                {
                    fprintf(stderr, "jsh: Syntax error : invalid usage of '%s'\n", arg);
                    jsh.last_exit_code = 1;
                    return NULL;
                }
                else
                {
                    target[j]->type = flags[j];
                    target[j]->path = strdup(parts[i + 1]);
                    i += 1;
                    redir = true;
                    break;
                }
            }
        }

        if (redir) continue;

        vector_append(&argv, strdup(parts[i]));
    }

    vector_append(&argv, NULL);
    vector_shrink(&argv);
    out->argc = vector_length(&argv) - 1;
    out->argv = (char **)argv.data;
    out->substitutions = substs;
    out->line = join_strings(partc, parts, " ");

    return out;
}


pipeline_t *parse_pipeline(int partc, char **parts){


    if (partc == 0) { return NULL;};

    pipeline_t *out = calloc(1, sizeof(pipeline_t));

    // Managing '&'
    for (int i = 0; i < partc - 1; ++i)
    {
        if (strcmp(parts[i], "&") == 0)
        {
            fprintf(stderr, "jsh: Syntax error : invalid usage of '&'\n");
            jsh.last_exit_code = 1;
            return NULL;
        }
    }
    if (strcmp(parts[partc - 1], "&") == 0)
    {
        out->background = true;
        partc--;
    }

    int start = 0;
    int end = 0;
    while (start < partc)
    {
        while (end < partc && strcmp(parts[end], "|") != 0)
            ++end;
        if (end < partc && !strcmp(parts[end], "|") && end - start < 1)
        {
            fprintf(stderr, "jsh: Syntax error : invalid usage of '|'\n");
            jsh.last_exit_code = 1;
            return NULL;
        }
        command_t *cmd = parse_command(end - start, &parts[start]);
        if (!cmd)
        {
            return NULL;
        }
        vector_append(&out->commands, cmd);
        start = end + 1;
        end = start;
    }

    vector_shrink(&out->commands);
    out->line = join_strings(partc, parts, " ");


    return out;
}

char *get_prompt()
{
    // Previous calculations
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    char curdir[strlen(pwd) + 1];
    getcwd(curdir, sizeof(curdir));
    long int nbjobs = job_count();
    unsigned int nbcj = nb_chiffres(nbjobs);

    // Formatting prompt
    char *prompt = calloc(64, 1);
    unsigned int lenprompt = 4 + nbcj; // 4 for "[]$␣" & nbcj for nbjobs
    if (strlen(curdir) + lenprompt > 30)
        sprintf(prompt, "%s[%ld]%s...%s%s$ ", cyan, nbjobs, vert, (curdir + (strlen(curdir) - 30 + lenprompt + 3)), normal);
    else
        sprintf(prompt, "%s[%ld]%s%s%s$ ", cyan, nbjobs, vert, curdir, normal);
    return prompt;
}

pipeline_t *read_pipeline()
{
    char *prompt = get_prompt();
    char *read = readline(prompt);
    free(prompt);
    if (read == NULL) { read = strdup("exit"); }

    char **parts = split_string(read, " ");
    int partc;
    for (partc = 0; parts[partc] != NULL; ++partc);

    pipeline_t *out = parse_pipeline(partc, parts);

    for (int i=0; parts[i] != NULL; ++i) free(parts[i]);
    free(parts);

    if (out != NULL) { add_history(read); }
    free(read);
    return out;
}
