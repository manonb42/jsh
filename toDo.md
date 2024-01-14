## Rendu

- [x] fichier `AUTHORS.md`
- [x] Makefile avec `make` et `make clean`
- [x] fichier `ARCHITECTURE.md` expliquant la stratégie adoptée pour répondre au sujet (notamment l'architecture logicielle, les structures de données et les algorithmes implémentés)

## Gestion des signaux

- [x] jsh ignore les signaux `SIGINT`, (...) contrairement aux processus exécutant des commandes internes. (**jsh.c:33-40** ; **exec.c:162-176**)
- [ ] OPTIONNEL: amélioration d'exit.

## Redirections

- [x] `<`,`>`,`>>`,`>|`,`2>`,`2>>`,`2>|` (**exec.c:111-150** ; **jsh.h:21**)
- [x] `|`
- [ ] `<()`

## Erreurs du parseur à gérer

- [x] `&` qui n'est pas à la fin (**input.c:64**)
- [x] `cmd > ` - redirection vers rien (**input.c:87**)
- [x] `cmd > | ...` - redirection vers pipe
- [x] `cmd | | ...` - double pipe

## Formatage du prompt

(**input.c:125-141**)

- [x] nombre de jobs surveillés avec une couleur
- [x] répertoire courant avec une couleur
- [x] 30 caractères max

## Gestion de la ligne de commande

- [x] affichage du prompt sur sa sortie erreur (**jsh.c:31**)
- [x] affichage du prompt et lecture de la ligne saisie (**jsh.c:50** ; **input.c:141**)
- [x] découpage en bloc (**input.c:153**)
- [x] interprétation des caractères spéciaux `&`,`<`,`>`,`|` (**input.c:**)
- [x] exécution de la commande (**jsh.c:52**)
- [x] check des jobs en cours avant chaque nouvel affichage du prompt (**jsh.c:48**)
- [x] affichage des jobs ayant changé de status (créé, stoppé, terminé, détaché) sauf pour ceux au premier plan devenus (achevé, tué ou détaché) (**jsh.c:49**)
- [x] Lorsque la fin de l'entrée standard est atteinte (Ctrl-D), exécute `exit` (**input.c:143**)

## Commandes internes

- [x] valeur de retour 0 pour succès et 1 pour échec
- [x] `cd`, `cd rep`, `cd -` (**internalcmd.c:38**)
- [x] `?` (**internalcmd.c:94**)
- [x] `exit` lorsqu'il y a des jobs en cours ou suspendus (**internalcmd.c:81**)
- [x] `exit val`, `exit` (**internalcmd.c:79**)
- [x] `jobs` (**internalcmd.c:107**)
- [x] `jobs -t`
- [x] `jobs %job`
- [x] `bg %job`, `fg %job` (**internalcmd.c:XXX**)
- [x] `kill [-sig] %job`, `kill [-sig] pid` (**internalcmd.c:122**)

## Commandes externes

- [x] exécuter toutes les commandes externes, avec ou sans arguments, en tenant compte de la variable d'environnement PATH. (**exec.c:172** ; **exec.c:80**, execv**p** prend en compte la variable PATH)

---

- [x] un job dont au moins un processus n'a pas terminé, mais dont tous les processus directement lancés par le shell ont terminé, est détaché (detached) et quitte la surveillance de jsh;
