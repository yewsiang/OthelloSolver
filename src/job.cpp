
#include "job.h"

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
				jobId, currentJob.id, 
				((OPP(currentJob.player) == BLACK) ? INT_MIN : INT_MAX), 
				currentJob.boardsAssessed + 1 
			};
			waitingJobs->push_back(waitingJob);

			jobId++;
		}
	}
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
	CompletedJob cj = {job->id, job->parentId, value, solver.getBoardsSearched()};
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


void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs) {
	printf("==== MASTER SENDING JOBS =========\n");

	int numJobs = jobs->size();
	for (int i = 1; i < numProcs; i++) {
		int problemSize = floor(numJobs * (i + 1) / numProcs) - floor(numJobs * i / numProcs);
		printf("For Processor %d, Problem size: %d\n", i, problemSize);

		vector<Job> jobsToSend;
		vector<Board> boardsToSend;
		for (int j = 0; j < problemSize; j++) {
			Job currentJob = jobs->front();
			jobs->pop_front();
			jobsToSend.push_back(currentJob);

			boardsToSend.push_back(boards->front());
			boards->pop_front();
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

void waitForJob(string jobType, int id) {
	if (jobType.compare("PARALLEL_MINIMAX") == 0) {
		vector<Job> jobsToWork;

		slaveReceiveJobs(&jobsToWork);
		
	    printf("Process %d received jobs from process 0:\n", id);
	    for (int i = 0; i < jobsToWork.size(); i++) {
	    	Job currentJob = jobsToWork[i];
	    	Board currentBoard = *(currentJob.board);

	    	printf("ID: %d [PLAYER: %d][LEFT: %d] \n", 
				currentJob.id, currentJob.player, currentJob.depthLeft);
	    	//currentBoard.printBoard(currentJob.player);
	    }
	
	    // Work on problems
	    vector<CompletedJob> completedJobs = executeAllJobs(jobsToWork);
	    for (int i = 0; i < completedJobs.size(); i++) {
  			printf("Process %d finished Job %d with Parent %d [Value: %d]\n", 
  				id, completedJobs[i].id, completedJobs[i].parentId, completedJobs[i].moveValue);
  		}
		
	    slaveSendCompletedJobs(&completedJobs);
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

		printf("Process 0 finished Job %d with Parent %d [Value: %d]\n", 
			completedJobs[i].id, completedJobs[i].parentId, completedJobs[i].moveValue);
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
