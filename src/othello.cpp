#include <iostream>
#include <mpi.h>
#include "config.h"
#include "game.h"
#include "job.h"

using namespace std;

int main(int argc, char** argv) {

	MPI_Init(NULL, NULL);
  	int numProcs, id;
  	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &id);

  	if (id == 0) {
		// Retrieve configurations
		Config cf = Config(argv[1], argv[2]);

		// Setup the game
		Game game = Game(cf);
		game.play(numProcs);

	} else {
		while (true) {
			slaveWaitForJob("PARALLEL_MINIMAX", id);
		}
	}

	MPI_Finalize();
}