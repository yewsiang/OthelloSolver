#include <iostream>
#include <mpi.h>
#include "config.h"
#include "game.h"
#include "job.h"

using namespace std;

string ALGORITHM = "JOBPOOL_MINIMAX";//"PARALLEL_MINIMAX";

int main(int argc, char** argv) {

	MPI_Init(NULL, NULL);
  	int numProcs, id;
  	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &id);

  	if (id == 0) {
  		cout << "Number of Processors: " << numProcs << endl;

		// Retrieve configurations
		Config cf = Config(argv[1], argv[2]);

		// Setup the game
		Game game = Game(cf);
		game.play(ALGORITHM, numProcs);

	} else {
		// Slave process acts differently depending on algorithm
		if (ALGORITHM.compare("PARALLEL_MINIMAX") == 0) {
			slaveWaitForJob(ALGORITHM, id);

		} else if (ALGORITHM.compare("JOBPOOL_MINIMAX") == 0) {
			slaveRequestJob(ALGORITHM, id);
		}

		printf("\nSLAVE %d FINALIZED\n", id);
	}

	MPI_Finalize();
}