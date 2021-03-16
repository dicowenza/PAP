export OMP_NUM_THREADS
export GOMP_CPU_AFFINITY

OUTPUT="transpose.data"


ITE=$(seq 3) # nombre de mesures

PARAM="../2Dcomp -n -k transpose -i 10  -s 4096 -g" # parametres commun à toutes les executions 


execute (){
EXE="$PARAM $*"
for nb in $ITE; do  echo -n "$* " >> $OUTPUT  ; $EXE 2>> $OUTPUT ; done
}

for i in 1 4 8 16 32 64 128 ; 
do
    execute $i -v tiled
done

