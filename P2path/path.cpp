#include "mpi.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <limits.h>
#include <iomanip>
#include <fstream>
#include <stdlib.h>

std::vector<int> generatePremutationByInedx(int index, std::vector<int> elements){
    std::vector<int> factorial;
    std::vector<int> out;
    int counter=1;

    while(index>0){
        factorial.push_back(index%counter);
        index=index/counter;
        counter++;
    }
    
    while(factorial.size()!=elements.size()){
        factorial.push_back(0);
    }
    
    for(int i=factorial.size()-1;i>=0;i--){
        out.push_back(elements[factorial[i]]);
        elements.erase(elements.begin()+factorial[i]);
    }
    
    return out;
}

int silnia (int n){
    if (n == 0) return 1;
    else return n*silnia(n-1);
}

int main(int argc, char *argv[]){
    int rank, size,rc,i,j;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if(argc<3){
        printf("Need 2 file names in args. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }
    int costs[10][10]= {{0,6,8,2,8,3,5,8,7,9},
                        {8,0,7,4,8,9,2,5,6,7},
                        {9,8,0,6,3,6,9,5,3,1},
                        {3,5,7,0,2,1,2,2,3,4},
                        {1,1,2,9,0,8,8,8,7,7},
                        {5,3,8,9,3,0,4,6,3,9},
                        {9,2,5,4,9,7,0,6,6,9},
                        {9,7,9,9,3,5,1,0,2,6},
                        {4,6,9,6,6,2,6,3,0,5},
                        {4,6,1,9,2,4,8,5,1,0}};
    std::vector<int> elements;
    for(i=0;i<10;i++)
        elements.push_back(i);
    
    int startPosition, howMany;    
    int index=-1;
    int cost=INT_MAX;
    int costTmp, indexTmp;
    double time1,time2;
    std::vector<int> out;
    
    if(rank==0){
        time1=MPI_Wtime();
        
        int allPermutations=silnia(10);
        
        for(i=1;i<size;i++){
            howMany=allPermutations/(size-1);
            startPosition=(i-1)*howMany;
            if(i==size-1){
                howMany=allPermutations/(size-1)+allPermutations%(size-1);
            }
           
            std::cout<<"Sending to rank="<<i<<" startPosition="<<startPosition<<" howMany="<<howMany<<std::endl;
            MPI_Send(&startPosition,1, MPI_INT, i,0,MPI_COMM_WORLD);
            MPI_Send(&howMany,1, MPI_INT, i,0,MPI_COMM_WORLD);
        }
        
        for(i=1;i<size;i++){
            std::cout<<"Reciving result from rank="<<i<<std::endl;
            MPI_Recv(&costTmp,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            MPI_Recv(&indexTmp,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            
            if(costTmp<cost){
                cost=costTmp;
                index=indexTmp;
            }
        }
        std::vector<int> out=generatePremutationByInedx(index,elements);
        time2=MPI_Wtime();
        
        std::ofstream file;
        file.open(argv[1]);
        file<<"cost="<<cost<<std::endl;
        file<<"index="<<index<<std::endl;
        file<<"path:"<<std::endl;
        for(i=0;i<out.size();i++){
            file<<out[i]<<std::endl;
        }
        file.close();
        
        file.open(argv[2],std::ofstream::out | std::ofstream::app);
        file<<std::setprecision(16)<<time2-time1<<std::endl;
        file.close();

    }
    else{
        MPI_Recv(&startPosition,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&howMany,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        
        #pragma omp parallel for shared(startPosition,howMany,cost,index,costs) private(i,j,out,costTmp)
        for(i=startPosition;i<startPosition+howMany;i++){
            out=generatePremutationByInedx(i,elements);
            costTmp=0;
            for(j=0;j<out.size()-1;j++){
                costTmp+=costs[out[j]][out[j+1]];
            }
            
            #pragma omp critical
            {
                if(costTmp<cost){
                    cost=costTmp;
                    index=i;
                }
            }
            
        }
        
        MPI_Send(&cost,1, MPI_INT, 0,0,MPI_COMM_WORLD);
        MPI_Send(&index,1, MPI_INT, 0,0,MPI_COMM_WORLD);
    }
   
    MPI_Finalize();
    
    return 0;
}