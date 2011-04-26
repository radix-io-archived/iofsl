#include "BenchmarkMain.h"
#include <string.h>

void LogResult ( FILE * f, char * s, double runTime, size_t outLen,
                char * filename)
{
  char * outStr = malloc(sizeof(char) * (strlen(s) + 45 + strlen(filename)));
  sprintf(outStr,"%s,%s,%10.8g,%i\n", s, filename, runTime, outLen);
  fwrite(outStr, sizeof(char), strlen(outStr), f);
  fflush(f);
  free (outStr);
}

void control (char * UNUSED(address), char * UNUSED(config), char * inDataset, 
              char * UNUSED(outDataset), int UNUSED(readSize), int  runs, char* csvOutput)
{
  FILE * logfile;
  double start, end;
  logfile = fopen(csvOutput, "a");
  /* skip the first run */
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD); 
  MPI_Barrier(MPI_COMM_WORLD);  
  MPI_Barrier(MPI_COMM_WORLD); 
  int i = 0;    
  for (i = 0; i < runs - 1; i++)
  {
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(logfile, "READ", end - start, 0, inDataset);
     MPI_Barrier(MPI_COMM_WORLD); 
     start = MPI_Wtime();
     MPI_Barrier(MPI_COMM_WORLD);
     end = MPI_Wtime();
     LogResult(logfile, "WRITE", end - start, 0, inDataset);

  }
}

int main(int argc, char *argv[]) 
{
    int runs = 21;
    if (argc < 4)
      return -1;
    char * address = argv[1];
    char * config = argv[2];
    char * inDataset = argv[3];
    char * outDataset = argv[4];
    int readSize = atoi(argv[5]);
    char * csvOutput = argv[6];    
    MPI_Init(0, NULL);           
    int rank, size; 
    MPI_Comm_size(MPI_COMM_WORLD, &size); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
    setenv("ZOIDFS_ION_NAME",address,1);
    if (rank == 0)
      control (address, config, inDataset, outDataset, readSize, runs, csvOutput);
    else
      test (address, config, inDataset, outDataset, readSize, runs);
    
    MPI_Finalize(); 
    return 0;
}

