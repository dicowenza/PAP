export OMP_NUM_THREADS
export GOMP_CPU_AFFINITY

ITE=$(seq 5) # nombre de mesures
THREADS="1 2 4 6 8 10 12 16 20 24" # nombre de threads à utiliser pour les expés
GOMP_CPU_AFFINITY=$(seq 0 23 ) # on fixe les threads 

PARAM="../tsp-main 13 1234 " # parametres commun à toutes les executions 

execute (){
EXE="$PARAM $*"
OUTPUT="$(echo $* | tr -d ' ')".data
for nb in $ITE; do for OMP_NUM_THREADS in $THREADS ; do echo -n "$OMP_NUM_THREADS " >> $OUTPUT  ; $EXE 2>> $OUTPUT ; done; done
}

#execute ompcol2
#execute ompcol3

THREADS="12"

for i in 1 2 3 ;  # grain 
do
    execute $i ompfor
done
