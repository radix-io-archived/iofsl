#include "BenchmarkMain.hh"
#include <iostream>
#include <fstream>
using namespace std;

void LogResult ( ofstream * f, char * s, double runTime, size_t outLen,
                char * filename)
{
  char * outStr = new char[strlen(s) + 45 + strlen(filename)];
  sprintf(outStr,"%s,%s,%10.8g,%i\n", s, filename, runTime, outLen);
  f->write(outStr, strlen(outStr));

  delete[] outStr;
}

void control (char * address, char * config, char * inDataset, 
              char * outDataset, int readSize, int  runs, char* csvOutput)
{
  ofstream logfile;
  double start, end;
  logfile.open(csvOutput, ios::app);
  /* skip the first run */
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD); 
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD);     
  for (int i = 0; i < runs - 1; i++)
  {
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(&logfile, "READ", end - start, 0, inDataset);
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(&logfile, "WRITE", end - start, 0, inDataset);
  }
}

int main(int argc, char *argv[]) 
{
    int runs = 21;
    if (argc < 4)
      return -1;

    int n, rank, size, i; 
    MPI::Init(argc, argv);           
    size = MPI::COMM_WORLD.Get_size(); 
    rank = MPI::COMM_WORLD.Get_rank(); 
    char * address = argv[1];
    char * config = argv[2];
    char * inDataset = argv[3];
    char * outDataset = argv[4];
    int readSize = std::atoi(argv[5]);
    char * csvOutput = argv[6];    
    if (rank == 0)
      control (address, config, inDataset, outDataset, readSize, runs, csvOutput);
    else
      test (address, config, inDataset, outDataset, readSize, runs);
    
    MPI::Finalize(); 
}

