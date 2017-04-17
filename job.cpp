
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

/******************************* JOB EXECUTION *******************************/

// Compute the minimax of each move of the board in a Job
CompletedJob executeMinimaxJob(Job* job) {
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

// Compute the minimax of each move of the board in a Job with alpha-beta pruning
CompletedJob executeAlphaBetaJob(Job* job) {
	Solver solver = Solver(job->width, job->height, job->depthLeft, 
		job->maxBoards, job->cornerValue, job->edgeValue);
	int player = job->player;
	int depth = job->depthLeft;
	Board* currentBoard = job->board;
	int value = (player == BLACK) ? solver.getAlphaBetaMaxValue(INT_MIN, INT_MAX, *currentBoard, player, depth) :
									solver.getAlphaBetaMinValue(INT_MIN, INT_MAX, *currentBoard, player, depth);
	CompletedJob cj = {job->id, job->parentId, player, value, solver.getBoardsSearched()};
	return cj;
}

vector<CompletedJob> executeAllJobs(string algorithm, vector<Job> job) {
	vector<CompletedJob> completedJobs;

	if (algorithm.compare("BATCH_MINIMAX") == 0 || 
		algorithm.compare("JOBPOOL_MINIMAX") == 0) {

		for (int i = 0; i < job.size(); i++) {
			CompletedJob cj = executeMinimaxJob(&job[i]);
			completedJobs.push_back(cj);
		}

	} else if (algorithm.compare("BATCH_ALPHABETA") == 0 ||
	   algorithm.compare("JOBPOOL_ALPHABETA") == 0) {

		for (int i = 0; i < job.size(); i++) {
			CompletedJob cj = executeAlphaBetaJob(&job[i]);
			completedJobs.push_back(cj);
		}
	}
	return completedJobs;
}

/********************************** GENERAL **********************************/

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

	// To prevent too many jobs from being created
	jobsPerProc = min(100, jobsPerProc);
	int jobId = jobs->size();

	// Split current Jobs into more Jobs until we reach desired Jobs per processor
	while (jobs->size() < numProcs * jobsPerProc) {
		// Get next Job
		Job currentJob = jobs->front();
		Board currentBoard = boards->front();
		jobs->pop_front();
		boards->pop_front();

		Solver solver = Solver(currentJob.width, currentJob.height, currentJob.depthLeft, 
			currentJob.maxBoards, currentJob.cornerValue, currentJob.edgeValue);

		// Do not send if the current board is over
		if (currentBoard.isGameOver()) {
			CompletedJob waitingJob = { 
				currentJob.id, currentJob.parentId, currentJob.player,
				solver.evaluateBoard(currentBoard), 
				currentJob.boardsAssessed + 1 
			};
			waitingJobs->push_back(waitingJob);

			jobId++;
			continue;
		}
		vector<point> validMoves = currentBoard.getValidMoves(currentJob.player);
		
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

void masterSendBatchJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs, string jobDistribution) {
	int numJobs = jobs->size();
	int jobsAllocated = 0;
	for (int i = 1; i < numProcs; i++) {
		int problemSize = floor(numJobs * (i + 1) / numProcs) - floor(numJobs * i / numProcs);
		printf("For Processor %d, Problem size: %d\n", i, problemSize);

		masterSendJobs(jobs, boards, i, problemSize, jobDistribution);
	}
}

void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int id, 
	int jobSize, string jobDistribution) {
	printf("==== MASTER SENDING %d JOBS =========\n", id);

	// Determine whether jobs to be sent are chosen randomly or sequentially
	bool randomizeJobDistribution = (jobDistribution.compare("RANDOM") == 0);

	// Prevent Job size from being bigger than the number of Jobs
	jobSize = min(jobSize, int(jobs->size()));
	
	int numJobs = jobs->size();
	int jobsAllocated = 0;
	vector<Job> jobsToSend;
	vector<Board> boardsToSend;
	for (int j = 0; j < jobSize; j++) {

		if (randomizeJobDistribution) {
			// Randomly choose the jobSize jobs
			srand(time(NULL));
			int randomId = rand() % (numJobs - jobsAllocated);
			Job currentJob = (*jobs)[randomId];
			jobs->erase(jobs->begin() + randomId);

			boardsToSend.push_back(boards->at(randomId));
			boards->erase(boards->begin() + randomId);
			jobsAllocated++;

			jobsToSend.push_back(currentJob);

		} else {
			// Choose the first Job that is available
			Job currentJob = jobs->front();
			jobs->erase(jobs->begin());

			boardsToSend.push_back(boards->front());
			boards->erase(boards->begin());

			jobsToSend.push_back(currentJob);
		}
	}

	// Send information
	MPI_Send((void*)jobsToSend.data(), jobsToSend.size() * sizeof(Job), 
		MPI_BYTE, id, 0, MPI_COMM_WORLD);
	
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

				MPI_Send(&num, 1, MPI_INT, id, uniqueTag, MPI_COMM_WORLD);
			}
		}
	}

	printf("==== MASTER SENDING JOBS ENDS ====\n");
}

void slaveReceiveJobs(vector<Job>* jobs) {
	// Probe for Jobs
	MPI_Status status;
	MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

	// Resize buffer based on how much data is being received
	int incomingSize;
	MPI_Get_count(&status, MPI_BYTE, &incomingSize);
	jobs->resize(incomingSize / sizeof(Job));

	// Receive configuration information of Jobs
	MPI_Recv((void*)jobs->data(), incomingSize, MPI_BYTE, 0, 0, MPI_COMM_WORLD, 
		MPI_STATUS_IGNORE);

	// If no incoming Jobs, simply return
	if (incomingSize == 0) {
		return;
	}

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

void slaveWaitForJob(string algorithm, int id) {
	// For timing purposes
	long long before, after;
	vector<Job> jobsToWork;

	// Receive Jobs from master
	before = wallClockTime();
	slaveReceiveJobs(&jobsToWork);
	after = wallClockTime();
	commTime += after - before;

	// Work on problems
	before = wallClockTime();
    vector<CompletedJob> completedJobs = executeAllJobs(algorithm, jobsToWork);
    after = wallClockTime();
	compTime += after - before;
	
	// Return results to master
	before = wallClockTime();
    slaveSendCompletedJobs(&completedJobs);
    after = wallClockTime();
	commTime += after - before;

	printf(" --- SLAVE %2d FINISHED: Communication =%6.2f s; Computation =%6.2f s\n", 
		id, commTime / 1000000000.0, compTime / 1000000000.0);
}

void masterWorkOnJobs(string algorithm, deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs) {
	// If no jobs, return
	if (int(jobs->size()) < 1) {
		return;
	}

	vector<Job> jobsToWork;
	for (int i = 0; i < jobs->size(); i++) {
		Job job = (*jobs)[i];
		job.board = &(*boards)[i];
		jobsToWork.push_back(job);
	}
	
    vector<CompletedJob> completedJobs = executeAllJobs(algorithm, jobsToWork);
    for (int i = 0; i < completedJobs.size(); i++) {
    	CompletedJob completedJob = completedJobs[i];
		int id = completedJob.id;
		int moveValue = completedJob.moveValue;
		int boardsAssessed = completedJob.boardsAssessed;

		(*waitingJobs)[id].moveValue = moveValue;
		(*waitingJobs)[id].boardsAssessed += boardsAssessed;
	}
}

void slaveSendCompletedJobs(vector<CompletedJob>* jobs) {
	// Send back to Processor 0 for results to be merged
	MPI_Send((void*)jobs->data(), jobs->size() * sizeof(CompletedJob), 
		MPI_BYTE, 0, 0, MPI_COMM_WORLD);
	jobs->clear();
}

void masterReceiveCompletedJobs(deque<CompletedJob>* waitingJobs, int numProcs) {
	vector<CompletedJob> incomingCompletedJobs;
	for (int i = 1; i < numProcs; i++) {
		// Probe for new incoming completed jobs
		MPI_Status status;
		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		// Resize your incoming walker buffer based on how much data is being received
		int incomingSize;
		MPI_Get_count(&status, MPI_BYTE, &incomingSize);	
		incomingCompletedJobs.resize(incomingSize / sizeof(CompletedJob));

		MPI_Recv((void*)incomingCompletedJobs.data(), incomingSize, MPI_BYTE, 
			status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// Add to completed jobs
		for (int j = 0; j < incomingCompletedJobs.size(); j++) {
			CompletedJob completedJob = incomingCompletedJobs[j];
			int id = completedJob.id;
			int moveValue = completedJob.moveValue;
			int boardsAssessed = completedJob.boardsAssessed;

			(*waitingJobs)[id].moveValue = moveValue;
			(*waitingJobs)[id].boardsAssessed += boardsAssessed;
		}
		incomingCompletedJobs.clear();
	}
}

void masterReceiveCompletedJobsFromSlave(deque<CompletedJob>* waitingJobs, int id) {
	vector<CompletedJob> incomingCompletedJobs;
	// Probe for new incoming completed jobs
	MPI_Status status;
	MPI_Probe(id, 0, MPI_COMM_WORLD, &status);

	// Resize your incoming walker buffer based on how much data is being received
	int incomingSize;
	MPI_Get_count(&status, MPI_BYTE, &incomingSize);	
	incomingCompletedJobs.resize(incomingSize / sizeof(CompletedJob));

	MPI_Recv((void*)incomingCompletedJobs.data(), incomingSize, MPI_BYTE, 
		status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	// Add to completed jobs
	for (int j = 0; j < incomingCompletedJobs.size(); j++) {
		CompletedJob completedJob = incomingCompletedJobs[j];
		int id = completedJob.id;
		int moveValue = completedJob.moveValue;
		int boardsAssessed = completedJob.boardsAssessed;

		(*waitingJobs)[id].moveValue = moveValue;
		(*waitingJobs)[id].boardsAssessed += boardsAssessed;
	}
	incomingCompletedJobs.clear();
}

/******************************** JOB POOLING ********************************/

// Receive Job requests from slaves and send some Jobs to slaves
void slaveRequestJob(string algorithm, int id) {
	// For timing purposes
	long long before, after;

	while (true) {
		int request = SLAVE_WANTS_JOBS;
		MPI_Send(&request, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

		MPI_Status status;
		int response;
		MPI_Recv(&response, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		if (response == MASTER_SENDING_JOBS) {
			// Receive Jobs from master
			before = wallClockTime();
			vector<Job> jobsToWork;
			slaveReceiveJobs(&jobsToWork);
			after = wallClockTime();
			commTime += after - before;

		    // Work on problems
			before = wallClockTime();
		    vector<CompletedJob> completedJobs = executeAllJobs(algorithm, jobsToWork);
		    after = wallClockTime();
			compTime += after - before;

			// Request to send Completed Jobs back to Master
			before = wallClockTime();
	  		request = SLAVE_SENDING_JOBS;
			MPI_Send(&request, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			after = wallClockTime();
			commTime += after - before;
			
			// Return results to master
			before = wallClockTime();
		    slaveSendCompletedJobs(&completedJobs);
		    after = wallClockTime();
			commTime += after - before;

		} else if (response == MASTER_NO_JOBS) {
			break;

		} else {
			// Error: Response must be either Master sending jobs or Master no jobs
			printf("--- ERROR ---\n");
			break;
		}
	}

	printf(" --- SLAVE %2d FINISHED: Communication =%6.2f s; Computation =%6.2f s\n", 
		id, commTime / 1000000000.0, compTime / 1000000000.0);
}

/*************************** COMBINATION OF RESULTS **************************/

// Combine evaluations by Slave processes to get minimax value for original moves
void masterRewindMinimaxStack(deque<CompletedJob>* jobs) {
	CompletedJob job = jobs->back();

	// While the completed job is not one of the original moves
	while (job.parentId != -1) {
		int parentId = job.parentId;
		int player = job.player;
		int moveValue = job.moveValue;
		int parentMoveValue = (*jobs)[parentId].moveValue;
		CompletedJob* parentJob = &((*jobs)[parentId]);
		parentJob->boardsAssessed += job.boardsAssessed;

		// Parent will choose to max (if it is BLACK) and min (if it is WHITE)
		parentJob->moveValue = (OPP(player) == BLACK) ? 
								max(parentMoveValue, moveValue) :
								min(parentMoveValue, moveValue);

		jobs->pop_back();
		job = jobs->back();
	}
}