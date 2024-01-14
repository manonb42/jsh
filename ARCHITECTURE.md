# JSH

## Description des fichiers

Nous avons développé notre code de façon modulaire, et avons donc découpé notre programme en plusieurs fichiers source. Chaque fichier **.c** est accompagné d'un fichier d'entête correspondant **.h** contenant les déclarations de fonctions du fichier **.c** ainsi que des définitions des différentes structures utilisées dans le programme.

### Fichiers .c

1.  **jsh.c :**

    - Fichier principal du programme, orchestrant le flux général du shell : il est responsable de la boucle principale d'exécution, la gestion des signaux, et de la mise à jour des processus.

2.  **jobs.c :**

    - Dédié à la gestion des jobs : il dispose notamment de fonctions permettant de définir et d'afficher l'état d'un job.

3.  **vector.c :**

    - Implémente une liste chainée polymorphe, permettant d'implémenter notre liste de processus et de jobs sans redondance.

4.  **input.c :**

    - Responsable de la gestion de l'entrée utilisateur, il gère la lecture des commandes : l'analyse syntaxique des commandes, la gestion des erreurs de saisie, ainsi que l'affichage du prompt.

5.  **exec.c :**

    - Gère l'exécution des commandes, il implémente la redirection d'entrées/sorties, l'exécution des pipes, l'exécution des commandes internes (fait appel à **internalcmd.c** dans ce cas) ou externes, en avant ou arrière plan.

6.  **internalcmd.c**
    - Gère l'éxecution des commandes internes

### Fichiers .h

1.  **jsh.h:**

    - Contient les définitions des structures correspondant aux commandes, aux pipelines, aux processus, et aux jobs.

2.  **jobs.h , vector.h, input.h, exec.h, internalcmd.h:**

    - Contiennent des définitions de fonctions.

## Gestion d'une commande

### Lecture

Cela se passe dans le fichier _input.c_

Premièrement, on affiche le prompt comme demandé dans l'énoncé, ensuite on lit la ligne entrée par l'utilisateur. Suite à ça on parse la commande, en vérifiant que la syntaxe est correcte, et on créé notre objet **pipeline_t**, contenant la liste des commandes à exécuter, un attribut correspondant à l'exécution en arrière-plan ou non, ainsi que la ligne entrée par l'utilisateur.

### Exécution

Cela se passe dans les fichiers _exec.c_ et _internalcmd.c_

Dans la fonction **exec_pipeline**, la première étape est d'initialiser un **job** correspondant à notre commande, contenant le **pgid**, **l'état**, et la **ligne** entrée par l'utilisateur. Suite à ça on initialise les potentiels tubes, et on exécute au fur et à mesure les différentes commandes via _exec_command_.
Les commandes internes sont définies à la main dans le fichier _internalcmd.c_.
Pour les fonctions externes, on utilise une fonction de la famille _exec_ couplée à un _fork_, et on met à jour le job.
Ensuite, on ajoute le job à jsh, et si la commande est à exécuter en avant-plan, on attend, sinon on passe à la suite.

### Suite

Quand une commande a terminé son exécution, on libère la mémoire allouée aux pointeurs utilisés, puis on recommence la boucle principale : on met à jour les jobs et on affiche ceux dont l'état a changé, puis on répète le processus de gestion de commande expliqué plus haut.
