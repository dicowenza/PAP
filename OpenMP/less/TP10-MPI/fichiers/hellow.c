/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include <stdio.h>
#include "mpi.h"

int main( int argc, char *argv[] )
{
    int rank;
    int size;
    MPI_Status status;
    char buf;
  

    /* initialiser MPI, à faire _toujours_ en tout premier du programme */
    MPI_Init(&argc,&argv);
    
    /* récupérer le nombre de processus lancés */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    /* récupérer son propre numéro */
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank==0){
        MPI_Send(&buf,1,MPI_CHAR,1,0,MPI_COMM_WORLD);
        printf( "envoie terminé\n" );
    }   
    else if (rank==1){
        MPI_Recv(&buf,1,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);
        printf( "reception terminée\n",&buf);
    }
        

    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();
    return 0;
}
