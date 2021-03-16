#!/usr/bin/env python3

from graphTools import *
from expTools import *
import os

# Dictionnaire avec les options de compilations d'apres commande
options = {}
options["-k "] = ["sable"]
options["-v "] = ["tiled", "FourWave","bool_FourWave", "tiled_bool_synch"]
options["-s "] = [480]
options["-a "] = ["4partout"]

# Pour renseigner l'option '-of' il faut donner le chemin depuis le fichier easypap
options["-of "] = ["./plots/data/perf_data.csv"]


# Dictionnaire avec les options OMP
ompenv = {}
ompenv["OMP_NUM_THREADS="] = [1] + list(range(8, 50, 8))
ompenv["OMP_PLACES="] = ["cores", "threads"]

nbrun = 1
# Lancement des experiences
execute('./run ', ompenv, options, nbrun, verbose=True, easyPath=".")

# Lancement de la version seq avec le nombre de thread impose a 1
options["-v "] = ["seq"]
ompenv["OMP_NUM_THREADS="] = [1]
execute('./run', ompenv, options, nbrun, verbose=False, easyPath=".")
