CAN
===

Implémentation basique du protocole CAN en MPI

Ce projet nécessite le package `cmake` pour compiler

### Fichiers de log ###

Après une exécution,
les fichiers de log se situent dans le répertoire `build/src/`

Il y a des images vectorielles en `.svg` créé à chaque étape d'invitation d'un nouveau noeud, et tout à la fin après la suppression d'un des noeuds de l'overlay

Puis, il y a aussi des fichiers de log en `.txt` créé à la fin de chaque étape (cf. `main.cpp`)

### Compilation seule ###

```shell
$ ./build.sh
```

ou

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Lancement ###

```shell
$ ./demo.sh
```
ou

```
$ mpirun -N [NB_PROCESS] ./build/src/exec [SEED]
```

`[SEED] :` graine passé à `srand` (ajouté au rank du process courant), argument optionnel, utile pour le debug


```
.
├── demo.sh               # Compile et lance une exécution du projet
├── build.sh              # Compile le projet seulement
├── README.md
├── src                   # Contient les sources du projet
│   ├── log.cpp
│   ├── log.hpp
│   ├── main.cpp
│   ├── utils.cpp
│   ├── utils.hpp
│   └── utils.tcc
└── test                  # Contient les tests unitaires (seulement test_zone.cpp)
    ├── _catch.hpp
    ├── catch.hpp
    ├── CMakeLists.txt
    └── test_zone.cpp
```
