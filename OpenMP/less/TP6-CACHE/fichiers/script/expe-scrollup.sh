export OMP_NUM_THREADS
export GOMP_CPU_AFFINITY

OUTPUT="scrollup.data"


ITE=$(seq 5) # nombre de mesures

PARAM="../2Dcomp -n -k scrollup -i 1000  -s" # parametres commun à toutes les executions 





execute (){
EXE="$PARAM $*"
for nb in $ITE; do  echo -n "$* " >> $OUTPUT  ; $EXE 2>> $OUTPUT ; done
}

# on suppose avoir codé 2 fonctions :
#   mandel_compute_omp_dynamic()
#   mandel_compute_omp_static()

for i in 16 32 48 64 80 96 112 128 256  ;  # 2 tailles : -s 256 puis -s 512 
do
    execute $i -v seq
    execute $i -v ji
   # execute $i -v seq2
done

