#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void asserting(int n)
{
    if (n < 0)
    {
        perror("redirectionner: asserting FAILED");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[])
{
    puts("");
    if (argc != 4)
    {
        fprintf(stderr, "Usage1: [cmd] [<, >, >|, >>, 2>, 2>|, 2>>] [fic]\n[cmd1] | [cmd2]\n");
        fprintf(stderr, "argc %d\n", argc);
        fprintf(stderr, "argv[0] = %s\targv[1] = %s\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    char *cmd = argv[1];
    char *dir = argv[2];
    char *fic = argv[3];
    // fd = descripteur du fichier
    // d = dup2
    // rd = redirection à faire
    // fst = début de la chaine à comparer
    int fd, d, rd, fst = 0;

    // Redirection à faire:
    if (dir[0] == '<')
        rd = STDIN_FILENO;
    else if (dir[0] == '2')
    {
        rd = STDERR_FILENO;
        fst = 1;
    }
    else
        rd = STDOUT_FILENO;

    // Mode d'ouverture du fichier
    if (!strcmp((dir + fst), "<"))
        fd = open(fic, O_RDONLY);
    else if (!strcmp((dir + fst), ">"))
        fd = open(fic, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    else if (!strcmp((dir + fst), ">|"))
        fd = open(fic, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    else if (!strcmp((dir + fst), ">>"))
        fd = open(fic, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    else
    {
        // Redirection not recognized;
        asserting(-1);
    }
    // On vérifie que tout s'est bien passé et on redirige la rd vers fd
    asserting(fd);
    d = dup2(fd, rd);
    asserting(d);
    close(fd);

    /* --------------------------------------------------------- */
    // Lancer test1.sh avec cette ligne:
    execlp(cmd, cmd, NULL);
    // Lancer test2.sh avec cette ligne:
    // execlp("cat", "cat", "tata", NULL);
    perror("redirectionner.c: erreur de commande");
    close(d);
    return 0;
}
