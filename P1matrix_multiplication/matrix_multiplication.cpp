#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

std::vector< std::vector< double > > load(std::string fileName){
    std::vector< std::vector< double> > out;
    std::ifstream file;
    file.open(fileName.c_str());
    std::string line;
    while (!file.eof()){
        std::getline(file, line);
        std::istringstream ss(line);
        double num;
        std::vector <double> nums;
        while(ss>>num){
            nums.push_back(num);
        }
        out.push_back(nums);
    }    
    file.close();
    
    return out;
}

int main (int argc, char *argv[])
{
    int	numtasks,              /* number of tasks in partition */
        taskid,                /* a task identifier */
        numworkers,            /* number of worker tasks */
        dest,                  /* task id of message destination */
        rows,                  /* rows of matrix A sent to each worker */
        averow, extra, offset, /* used to determine rows sent to each worker */
        i, j, k, rc;           /* misc */

    int NRA,/* number of rows in matrix A */
        NCA,/* number of columns in matrix A */
        NCB;/* number of columns in matrix B */               
                       
    std::vector< std::vector< double> > a; /* matrix A to be multiplied */
    std::vector< std::vector< double> > b; /* matrix B to be multiplied */  
    std::vector< std::vector< double> > c; /* result matrix C */

    double time1,time2; 

    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
    if (numtasks < 2 ) {
      printf("Need at least two MPI tasks. Quitting...\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      exit(1);
      }
    numworkers = numtasks-1;


/**************************** master task ************************************/
   if (taskid == MASTER)
   {
      printf("matrix mulitplication has started with %d tasks.\n",numtasks);
      printf("Initializing arrays...\n");
      
      if(argc<5){
        printf("Need 4 file names in args. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
      }
      
      a=load(argv[1]);
      b=load(argv[2]);
      
      NRA=a.size();
      NCA=a[0].size();
      NCB=b[0].size(); 
      
      time1=MPI_Wtime();
      
    } 
   
    MPI_Bcast(&NRA, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast(&NCA, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast(&NCB, 1, MPI_INT, 0, MPI_COMM_WORLD );
         
   if (taskid == MASTER)
   {       
      /* Send matrix data to the worker tasks */
      averow = NRA/numworkers;
      extra = NRA%numworkers;
      offset = 0;
      
      for (dest=1; dest<=numworkers; dest++)
      {
         rows = (dest <= extra) ? averow+1 : averow;   	
         printf("Sending %d rows to task %d offset=%d\n",rows,dest,offset);
         MPI_Send(&offset, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
         MPI_Send(&rows, 1, MPI_INT, dest, FROM_MASTER, MPI_COMM_WORLD);
         for(i=0;i<rows;i++) MPI_Send(&a[offset+i][0], NCA, MPI_DOUBLE, dest, FROM_MASTER, MPI_COMM_WORLD);
         for(i=0;i<NCA;i++) MPI_Send(&b[i][0], NCB, MPI_DOUBLE, dest, FROM_MASTER, MPI_COMM_WORLD);
         
         offset = offset + rows;
      }

      c.resize(NRA);
      #pragma omp parallel for shared(NCB,c) private(i)
      for(i=0;i<c.size();i++) c[i].resize(NCB); 
      
      /* Receive results from worker tasks */
      for (i=1; i<=numworkers; i++)
      {
         MPI_Recv(&offset, 1, MPI_INT, i, FROM_WORKER, MPI_COMM_WORLD, &status);
         MPI_Recv(&rows, 1, MPI_INT, i, FROM_WORKER, MPI_COMM_WORLD, &status);
         for(j=0;j<rows;j++) MPI_Recv(&c[offset+j][0], NCB, MPI_DOUBLE, i, FROM_WORKER, MPI_COMM_WORLD, &status);
         printf("Received results from task %d\n",i);
      }
      
      time2=MPI_Wtime();

      std::ofstream file;
      file.open(argv[3]);
      for (i=0; i<NRA; i++)
      {
         for (j=0; j<NCB; j++){
            file<<c[i][j]<<" ";
         }
         file<<"\n";
      }
      file.close();
      
      file.open(argv[4],std::ofstream::out | std::ofstream::app);
      file<<std::setprecision(16)<<time2-time1<<std::endl;
      file.close();
      
      printf ("Done.\n");
   }


/**************************** worker task ************************************/
   if (taskid > MASTER)
   {
      MPI_Recv(&offset, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
      
      a.resize(rows);
      #pragma omp parallel for shared(a,NCA) private(i)
      for(i=0;i<a.size();i++) a[i].resize(NCA);
   
      b.resize(NCA);
      #pragma omp parallel for shared(b,NCB) private(i)
      for(i=0;i<b.size();i++) b[i].resize(NCB);
   
      c.resize(rows);
      #pragma omp parallel for shared(c,NCB) private(i,j)
      for(i=0;i<c.size();i++){
        c[i].resize(NCB);
        for(j=0;j<c[i].size();j++){
            c[i][j]=0.0;
        }
      }
      
      for(i=0;i<rows;i++) MPI_Recv(&a[i][0], NCA, MPI_DOUBLE, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
      for(i=0;i<NCA;i++) MPI_Recv(&b[i][0], NCB, MPI_DOUBLE, MASTER, FROM_MASTER, MPI_COMM_WORLD, &status);
      
      #pragma omp parallel for shared(a,b,c,NCB,rows,NCA) private(k,i,j)
      for (k=0; k<NCB; k++){
         for (i=0; i<rows; i++)
         {
            for (j=0; j<NCA; j++){
               #pragma omp atomic
               c[i][k] = c[i][k] + a[i][j] * b[j][k];
            }
         }
      }
  
      MPI_Send(&offset, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, MASTER, FROM_WORKER, MPI_COMM_WORLD);
      for(i=0;i<rows;i++)  MPI_Send(&c[i][0], NCB, MPI_DOUBLE, MASTER, FROM_WORKER, MPI_COMM_WORLD);
   }
   MPI_Finalize();
}
