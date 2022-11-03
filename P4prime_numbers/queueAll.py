# -*- encoding: utf-8 -*-
import sys
import subprocess
if __name__ == "__main__": 
    runCount=100
    processesCountList=[2,3,4,5,6,7,8,9,10,25,50,100,150,200]
    hostsList=[1,2,3]
    taskList=[
    "python taskQueue.py "+str(runCount)+" mpiexec -np [x] -hostfile hosts_[y].txt prog_mpi.exe 150000 result_mpi_[y]_[x].txt time_mpi_[y]_[x].txt",
    "python taskQueue.py "+str(runCount)+" mpiexec -np [x] -hostfile hosts_[y].txt prog_mpi_openmp.exe 150000 result_mpi_openmp_[y]_[x].txt time_mpi_openmp_[y]_[x].txt",
    ]
    
    for i in taskList:
        for j in hostsList:
            for k in processesCountList:
                cmd=i.replace("[x]",str(k)).replace("[y]", str(j))
                print cmd
                print "Running cmd="+cmd
                process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
                print "Waiting cmd="+cmd
                process.wait()
                print "End cmd="+cmd
                

        