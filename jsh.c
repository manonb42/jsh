#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
	char** res = split_string("ls -la ."," ");
	execvp(res[0], res+1);

}
