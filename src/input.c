#include "input.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

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

int nbchiffres(int nb)
{
    int i = 1;
    while (nb != 0)
    {
        nb /= 10;
        ++i;
    }
    return i;
}

command_t *parse_command(char *read){
    command_t *out = malloc(sizeof(command_t));
    char **argv = split_string(read, " ");
    int argc;
    bool bg = false;
	//faudra changer la valeur mais bon pour l'instant ça va
	char **cmd = calloc(512, sizeof(char *));
	char *redir = "";
	char *fic = "";
	int descr;
    for (argc = 0; argv[argc] != NULL; ++argc)
    {		
		if (strcmp(argv[argc], "<") == 0 && argv[argc + 1] != NULL) {
			redir = "<";
			fic = argv[argc + 1];
			descr = STDIN_FILENO;
			break;
		}
		if (strcmp(argv[argc], "2>") == 0 && argv[argc + 1] != NULL) {
			redir = ">";
			fic = argv[argc + 1];
			descr = STDERR_FILENO;
			break;
		}
		if (strcmp(argv[argc], "2>|") == 0 && argv[argc + 1] != NULL) {
			redir = ">|";
			fic = argv[argc + 1];
			descr = STDERR_FILENO;
			break;
		}
		if (strcmp(argv[argc], "2>>") == 0 && argv[argc + 1] != NULL) {
			redir = ">>";
			fic = argv[argc + 1];
			descr = STDERR_FILENO;
			break;
		}
		if (strcmp(argv[argc], ">") == 0 && argv[argc + 1] != NULL) {
			redir = ">";
			fic = argv[argc + 1];
			descr = STDOUT_FILENO;
			break;
		}
		if (strcmp(argv[argc], ">|") == 0 && argv[argc + 1] != NULL) {
			redir = ">|";
			fic = argv[argc + 1];
			descr = STDOUT_FILENO;
			break;
		}
		if (strcmp(argv[argc], ">>") == 0 && argv[argc + 1] != NULL) {
			redir = ">>";
			fic = argv[argc + 1];
			descr = STDOUT_FILENO;
			break;
		}

        if (strcmp(argv[argc], "&") == 0 && argv[argc + 1] == NULL)
        {
            argv[argc] = NULL;
            bg = true;
        }
		cmd[argc] = argv[argc];
    }
	bool is_redir = strlen(redir) > 0;
	*out = (command_t){.argc = argc, .is_redir = is_redir,.descr = descr, .redir = redir, .fic = fic, .argv = cmd, .bg = bg};
    return out;
}

command_t *read_command()
{
    // Previous calculations
    char pwd[1024];
    getcwd(pwd, sizeof(pwd));
    char curdir[strlen(pwd) + 1];
    getcwd(curdir, sizeof(curdir));
    long int nbjobs = vector_length(&jsh.processes);
    unsigned int nbcj = nbchiffres(nbjobs);

    // Formatted prompt
    char prompt[30];
    unsigned int lenprompt = 4 + nbcj; // 4 for "[]$ " & nbcj for nbjobs
    if (strlen(curdir) + lenprompt > 30)
        sprintf(prompt, "%s[%ld]%s...%s%s$ ", cyan, nbjobs, vert, (curdir + (strlen(curdir) - 30 + lenprompt + 3)), normal);
    else
        sprintf(prompt, "%s[%ld]%s%s%s$ ", cyan, nbjobs, vert, curdir, normal);

    char *read = readline(prompt);
    if (read == NULL) { return parse_command("exit");}
    if (strcmp(read, "") == 0) { free(read); return NULL; }
    add_history(read);

    command_t *out = parse_command(read);
    free(read);
    return out;
}
