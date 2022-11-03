#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <vector>

int main(int argc, char *argv[]){
    int rank, size,rc,i,j,howManyAll,howMany,startPosition,counter,tmpInt,allPrimeNumbers=0;
    double time1,time2;
    std::vector<int> primeNumbers;
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
            startPosition=(i-1)*howMany;
            if(i==size-1){
                howMany=howManyAll/(size-1)+howManyAll%(size-1);
            }
           
            std::cout<<"Sending to rank="<<i<<" startPosition="<<startPosition<<" howMany="<<howMany<<std::endl;
            MPI_Send(&startPosition,1, MPI_INT, i,0,MPI_COMM_WORLD);
            MPI_Send(&howMany,1, MPI_INT, i,0,MPI_COMM_WORLD);
        }
        
        for(i=1;i<size;i++){
            std::cout<<"Reciving from rank="<<i<<std::endl;
            MPI_Recv(&tmpInt,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            primeNumbers.resize(allPrimeNumbers+tmpInt);
            MPI_Recv(&primeNumbers[allPrimeNumbers],tmpInt,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            allPrimeNumbers+=tmpInt;
        }
        
        time2=MPI_Wtime();
        
        std::ofstream file;
        file.open(argv[2]);
        for(int i=0;i<primeNumbers.size();i++){
            file<<primeNumbers[i]<<std::endl;
        }
        file.close();
        
        file.open(argv[3],std::ofstream::out | std::ofstream::app);
        file<<std::setprecision(16)<<time2-time1<<std::endl;
        file.close();

    }
    else{
        MPI_Recv(&startPosition,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&howMany,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        
        #pragma omp parallel for shared(startPosition,howMany) private(i,j,counter)
        for(i=startPosition;i<startPosition+howMany;i++){
            if (i<=1) continue;
            counter=0;
            for(j=1;j<=i;j++){
                if(i%j==0) counter++;
                if(counter>2) break;
            }
            
            if(counter==2){
                #pragma omp critical
                primeNumbers.push_back(i);
            }
        }
        tmpInt=primeNumbers.size();
        MPI_Send(&tmpInt,1, MPI_INT, 0,0,MPI_COMM_WORLD);
        MPI_Send(&primeNumbers[0],tmpInt, MPI_INT, 0,0,MPI_COMM_WORLD);
    }
   
    MPI_Finalize();
    
    return 0;
}