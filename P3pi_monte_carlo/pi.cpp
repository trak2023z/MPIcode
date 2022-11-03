#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <fstream>
#include <stdio.h>
#include <iomanip>

int main(int argc, char *argv[]){
    int rank, size,rc,i,howManyAll,howMany,count=0,tmp;
    double x,y,z,pi,time1,time2;
    srand(35791246);
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if(argc<4){
        printf("Need 3 parameters in args. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }
    
    if(rank==0){
        time1=MPI_Wtime();
        howManyAll=atoi(argv[1]);
      
        for(i=1;i<size;i++){
            howMany=howManyAll/(size-1);
            if(i==size-1){
                howMany=howManyAll/(size-1)+howManyAll%(size-1);
            }
           
            std::cout<<"Sending to rank="<<i<<" howMany="<<howMany<<std::endl;
            MPI_Send(&howMany,1, MPI_INT, i,0,MPI_COMM_WORLD);
        }
        
        for(i=1;i<size;i++){
            std::cout<<"Reciving from rank="<<i<<std::endl;
            MPI_Recv(&tmp,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            count+=tmp;
        }
        
        pi=((double)count/(double)howManyAll)*4.0; 
        
        time2=MPI_Wtime();
        
        std::ofstream file;
        file.open(argv[2]);
        file<<"pi="<<pi<<std::endl;
        file.close();
        
        file.open(argv[3],std::ofstream::out | std::ofstream::app);
        file<<std::setprecision(16)<<time2-time1<<std::endl;
        file.close();

    }
    else{
        MPI_Recv(&howMany,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        
        #pragma omp parallel for shared(howMany,count) private(i,x,y,z)
        for (i=0; i<howMany; ++i){                 
            x=((double)rand())/RAND_MAX;           //gets a random x coordinate
            y=((double)rand())/RAND_MAX;           //gets a random y coordinate
            z = sqrt(x*x+y*y);                  //Checks to see if number in inside unit circle
            if (z<=1){
                #pragma omp atomic
                count++;                //if it is, consider it a valid random point
            }
        }
        
        MPI_Send(&count,1, MPI_INT, 0,0,MPI_COMM_WORLD);
    }
   
    MPI_Finalize();
    
    return 0;
}