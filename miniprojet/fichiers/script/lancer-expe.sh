export OMP_NUM_THREADS
export GOMP_CPU_AFFINITY

ITE=$(seq 3) # nombre de mesures
THREADS="1 4 8 12 16 24" # nombre de threads à utiliser pour les expés
GOMP_CPU_AFFINITY=$(seq 0 23) # on fixe les threads

PARAM="./2Dcomp -n -k scrollup -i 100 -g 16 -s " # parametres commun à toutes les executions 

execute (){
EXE="$PARAM $*"
OUTPUT="$(echo $* | tr -d ' ')".data
for nb in $ITE; do for OMP_NUM_THREADS in $THREADS ; do echo -n "$OMP_NUM_THREADS " >> $OUTPUT  ; $EXE 2>> $OUTPUT ; done; done
}

# on suppose avoir codé 2 fonctions :
#   mandel_compute_omp_dynamic()
#   mandel_compute_omp_static()

for i in  16 16 256 ;  # 2 tailles : -s 256 puis -s 512 
do
	     execute  $i -v ji
	     execute  $i -v seq
		 #execute  $i -v ompd_k
		 #execute  $i -v omptiled
done

