
#include "job.h"

// For timing purposes (From "mm-mpi.c")
long long commTime = 0;
long long compTime = 0;
long long wallClockTime() {
#ifdef LINUX
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);
	return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

// Compute the minimaxScores of each move of the board in a Job
CompletedJob executeJob(Job* job) {
	Solver solver = Solver(job->width, job->height, job->depthLeft, 
		job->maxBoards, job->cornerValue, job->edgeValue);
	int player = job->player;
	int depth = job->depthLeft;
	Board* currentBoard = job->board;
	int value = (player == BLACK) ? solver.getMaxValue(*currentBoard, player, depth) :
									solver.getMinValue(*currentBoard, player, depth);
	CompletedJob cj = {job->id, job->parentId, player, value, solver.getBoardsSearched()};
	return cj;
}

vector<CompletedJob> executeAllJobs(vector<Job> job) {
	vector<CompletedJob> completedJobs;
	for (int i = 0; i < job.size(); i++) {
		CompletedJob cj = executeJob(&job[i]);
		completedJobs.push_back(cj);
	}
	return completedJobs;
}


void masterInitialiseJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs, 
	vector<point> validMoves, Board board, int player, int depth, int maxBoards, int cornerValue, int edgeValue) {
	int width = board.getWidth();
	int height = board.getHeight();
	for (int i = 0; i < validMoves.size(); i++) {
		Board newBoard = board;
		newBoard.makeMove(player, validMoves[i].x, validMoves[i].y);		

		// Setup jobs. parentId = -1 since they are the original moves
		Job newJob = {
			i, -1, width, height, maxBoards, cornerValue, edgeValue, 
			OPP(player), depth - 1, 0, &newBoard
		};
		jobs->push_back(newJob);
		boards->push_back(newBoard);

		// Setup waiting jobs to combine results when Slaves are done
		CompletedJob waitingJob = {
			i, -1, OPP(player), ((OPP(player) == BLACK) ? INT_MIN : INT_MAX), 0
		};
		waitingJobs->push_back(waitingJob);
	}
}

// Given a current Job, split into multiple Jobs by taking the first valid move in the board.
// Parent jobs are stored in waitingJobs so that they can be recombined when Slaves are done.
void splitJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs, 
	int numProcs, int jobsPerProc) {
	int jobId = jobs->size();
	// Split current Jobs into more Jobs until we reach desired Jobs per processor
	while (jobs->size() < numProcs * jobsPerProc) {
		// Get next Job
		Job currentJob = jobs->front();
		Board currentBoard = boards->front();
		jobs->pop_front();
		boards->pop_front();

		vector<point> validMoves = currentBoard.getValidMoves(currentJob.player);
		//if (validMoves.size() == 0) {

			//}
		for (int i = 0; i < validMoves.size(); i++) {
			Board newBoard = currentBoard;
			point move = validMoves[i];
			newBoard.makeMove(currentJob.player, move.x, move.y);

			// Package into Job and send it back into Job queue
			Job newJob = {
				jobId, currentJob.id, currentJob.width, currentJob.height, 
				currentJob.maxBoards, currentJob.cornerValue, currentJob.edgeValue,
				OPP(currentJob.player), currentJob.depthLeft - 1, 
				currentJob.boardsAssessed + 1, &newBoard
			};
			jobs->push_back(newJob);
			boards->push_back(newBoard);

			// Update waiting Jobs
			CompletedJob waitingJob = { 
				jobId, currentJob.id, OPP(currentJob.player),
				((OPP(currentJob.player) == BLACK) ? INT_MIN : INT_MAX), 
				currentJob.boardsAssessed + 1 
			};
			waitingJobs->push_back(waitingJob);

			jobId++;
		}
	}
}

void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs) {
	printf("==== MASTER SENDING JOBS =========\n");

	int numJobs = jobs->size();
	int jobsAllocated = 0;
	for (int i = 1; i < numProcs; i++) {
		int problemSize = floor(numJobs * (i + 1) / numProcs) - floor(numJobs * i / numProcs);
		printf("For Processor %d, Problem size: %d\n", i, problemSize);

		vector<Job> jobsToSend;
		vector<Board> boardsToSend;
		for (int j = 0; j < problemSize; j++) {
			Job currentJob = jobs->front();
			jobs->pop_front();

			boardsToSend.push_back(boards->front());
			boards->pop_front();

			/*srand(time(NULL));
			int randomId = rand() % (numJobs - jobsAllocated);
			Job currentJob = (*jobs)[randomId];
			jobs->erase(jobs->begin() + randomId);

			boardsToSend.push_back(boards->at(randomId));
			boards->erase(boards->begin() + randomId);
			jobsAllocated++;*/

			jobsToSend.push_back(currentJob);
		}

		// Send information
		MPI_Send((void*)jobsToSend.data(), jobsToSend.size() * sizeof(Job), 
			MPI_BYTE, i, 0, MPI_COMM_WORLD);
		
		// Send array data
		int width = jobs->front().width;
		int height = jobs->front().height;
		for (int k = 0; k < boardsToSend.size(); k++) {
			Board currentBoard = boardsToSend[k];
			
			for (int w = 0; w < width; w++) {
				for (int h = 0; h < height; h++) {
					// (+ 1) needed at the end because 0 was used to send just now
					int uniqueTag = (w * width + h) + (k * width * height) + 1;
					int num = currentBoard.getDisk(w, h);

					MPI_Send(&num, 1, MPI_INT, i, uniqueTag, MPI_COMM_WORLD);
				}
			}
		}
	}

	printf("==== MASTER SENDING JOBS ENDS ====\n");
}

void slaveReceiveJobs(vector<Job>* jobs) {
	// Probe for new incoming walkers
	MPI_Status status;
	MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

	// Resize your incoming walker buffer based on how much data is being received
	int incomingSize;
	MPI_Get_count(&status, MPI_BYTE, &incomingSize);
	jobs->resize(incomingSize / sizeof(Job));

	// Receive configuration information of Jobs
	MPI_Recv((void*)jobs->data(), incomingSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, 
		MPI_STATUS_IGNORE);

	// Receive array data of Jobs
	int width = jobs->front().width;
	int height = jobs->front().height;
	for (int k = 0; k < jobs->size(); k++) {
		Job* currentJob = &((*jobs)[k]);
		Board* newBoard = new Board(width, height);
		int data[width][height];

		for (int w = 0; w < width; w++) {
			for (int h = 0; h < height; h++) {
				// (+ 1) needed at the end because 0 was used to send just now
				int uniqueTag = (w * width + h) + (k * width * height) + 1;
				
				MPI_Recv(&data[w][h], 1, MPI_INT, 0, uniqueTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}

		// Setup new board
		for (int w = 0; w < width; w++) {
			for (int h = 0; h < height; h++) {
				newBoard->setDisk(data[w][h], w, h);
			}
		}
		currentJob->board = newBoard;
	}
}

void slaveWaitForJob(string jobType, int id) {
	// For timing purposes
	long long before, after;

	if (jobType.compare("PARALLEL_MINIMAX") == 0) {
		vector<Job> jobsToWork;

		// Receive Jobs from master
		before = wallClockTime();
		slaveReceiveJobs(&jobsToWork);
		after = wallClockTime();
		commTime += after - before;
	    /*printf("Process %d received jobs from process 0:\n", id);
	    for (int i = 0; i < jobsToWork.size(); i++) {
	    	Job currentJob = jobsToWork[i];
	    	Board currentBoard = *(currentJob.board);

	    	printf("ID: %d [PLAYER: %d][LEFT: %d] \n", 
				currentJob.id, currentJob.player, currentJob.depthLeft);
	    }*/
	
		// Work on problems
		before = wallClockTime();
	    vector<CompletedJob> completedJobs = executeAllJobs(jobsToWork);
	    after = wallClockTime();
		compTime += after - before;
	    /*for (int i = 0; i < completedJobs.size(); i++) {
  			printf("Process %d finished Job %d with Parent %d [Value: %d]\n", 
  				id, completedJobs[i].id, completedJobs[i].parentId, completedJobs[i].moveValue);
  		}*/
		
		// Return results to master
		before = wallClockTime();
	    slaveSendCompletedJobs(&completedJobs);
	    after = wallClockTime();
		commTime += after - before;

		printf(" --- SLAVE %2d FINISHED: Communication =%6.2f s; Computation =%6.2f s\n", 
			id, commTime / 1000000000.0, compTime / 1000000000.0);
	}
}

void masterWorkOnJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs) {
	vector<Job> jobsToWork;
	for (int i = 0; i < jobs->size(); i++) {
		Job job = (*jobs)[i];
		job.board = &(*boards)[i];
		jobsToWork.push_back(job);
	}
	
    vector<CompletedJob> completedJobs = executeAllJobs(jobsToWork);
    for (int i = 0; i < completedJobs.size(); i++) {
    	CompletedJob completedJob = completedJobs[i];
		int id = completedJob.id;
		int moveValue = completedJob.moveValue;
		int boardsAssessed = completedJob.boardsAssessed;

		(*waitingJobs)[id].moveValue = moveValue;
		(*waitingJobs)[id].boardsAssessed += boardsAssessed;

		//printf("Process 0 finished Job %d with Parent %d [Value: %d]\n", 
		//	completedJobs[i].id, completedJobs[i].parentId, completedJobs[i].moveValue);
	}
}

void slaveSendCompletedJobs(vector<CompletedJob>* jobs) {
	// Send back to Processor 0 for results to be merged
	MPI_Send((void*)jobs->data(), jobs->size() * sizeof(CompletedJob), 
		MPI_BYTE, 0, 0, MPI_COMM_WORLD);
	jobs->clear();
}

void masterReceiveCompletedJobs(deque<CompletedJob>* waitingJobs, int numProcs) {
	for (int i = 1; i < numProcs; i++) {
		vector<CompletedJob> incomingCompletedJobs;

		// Probe for new incoming walkers
		MPI_Status status;
		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		// Resize your incoming walker buffer based on how much data is being received
		int incoming_size;
		MPI_Get_count(&status, MPI_BYTE, &incoming_size);
		incomingCompletedJobs.resize(incoming_size / sizeof(CompletedJob));

		MPI_Recv((void*)incomingCompletedJobs.data(), incoming_size, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, 
			MPI_STATUS_IGNORE);
		// Add to completed jobs
		for (int j = 0; j < incomingCompletedJobs.size(); j++) {
			CompletedJob completedJob = incomingCompletedJobs[j];
			int id = completedJob.id;
			int moveValue = completedJob.moveValue;
			int boardsAssessed = completedJob.boardsAssessed;

			//waitingJobs->push_back(currentJob);
			(*waitingJobs)[id].moveValue = moveValue;
			(*waitingJobs)[id].boardsAssessed += boardsAssessed;
		}
		incomingCompletedJobs.clear();
	}
}

// Combine evaluations by Slave processes to get minimax value for original moves
void masterRewindMinimaxStack(deque<CompletedJob>* jobs) {
	CompletedJob job = jobs->back();

	// While the completed job is not one of the original moves
	while (job.parentId != -1) {
		int parentId = job.parentId;
		int player = job.player;
		int moveValue = job.moveValue;
		int parentMoveValue = (*jobs)[parentId].moveValue;
		//printf("REWIND [PARENT ID: %d] [PLAYER: %s] [VALUE: %d][PARENT VALUE: %d]\n",
		//	parentId, (player == BLACK) ? "BLACK" : "WHITE", moveValue, parentMoveValue);
		CompletedJob* parentJob = &((*jobs)[parentId]);
		// Parent will choose to max (if it is BLACK) and min (if it is WHITE)
		parentJob->moveValue = (OPP(player) == BLACK) ? 
								max(parentMoveValue, moveValue) :
								min(parentMoveValue, moveValue);
		jobs->pop_back();
		job = jobs->back();
	}
}