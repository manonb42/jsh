#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <wait.h>


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


int main () {
    char *read;
    char **split;

    int status;
    int return_code = 0;

    char buf[1024];

    rl_initialize();
    rl_outstream = stderr;
    while(1) {
        read = readline(">");
        add_history(read);
        split = split_string(read," ");


        if (strcmp (read,"?") == 0){
            printf("%d\n", return_code);
        }
        else if (strcmp (read,"pwd") == 0){
            if(getcwd(buf, sizeof(buf)) != NULL) {
                printf("%s\n", buf);
            }
            printf("pwd Failure\n");
        }
        if (fork() == 0) {
            execvp(split[0], split);
        } else {
            waitpid(0, &status, 0);
            return_code = WEXITSTATUS(status);
        }
    }

    free(buf);
    free(read);
    free(split);
    return 0;
}


