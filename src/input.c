#include "input.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

char **split_string(char *chemin, char* separateur){
    char *chem = strdup(chemin);
    int size = 1;
    for(int i=0; i<strlen(chem); ++i)
        if(chem[i]==separateur[0]) ++size;
    char** composantes = calloc(size+2, sizeof(char*));
    int i=0;
    for(char* mot = strtok(chem, separateur); mot!= NULL; mot = strtok(NULL, separateur)){
        composantes[i]=strdup(mot);
        i++;
    }
    free(chem);
    composantes[size + 1] = NULL;
    return composantes;
}


command_t *read_command(){
    char *read = readline(">");
    if (read == NULL) return NULL;
    add_history(read);
    char **argv = split_string(read," ");
    int argc;
    for (argc=0; argv[argc] != NULL; ++argc);

    command_t *out = malloc(sizeof(command_t));
    *out = (command_t){ .argc = argc, .argv = argv};
    return out;
}
