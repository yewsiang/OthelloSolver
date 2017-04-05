
#include "job.h"

unsigned long getSizeOfJob(int width, int height) {
	unsigned long sizeOfParams = 6 * sizeof(int);
	unsigned long sizeOfBoard = (width * height) * sizeof(int) + sizeof(int*);
	return sizeOfParams + sizeOfBoard;
}

// Given a current Job, split into multiple Jobs by taking the first valid move in the board
vector<Job> splitJob(Job job) {
	vector<Job> jobs;
	return jobs;
}

// Compute the minimaxScores of each move of the board in a Job
CompletedJob executeJob(Job* job) {
	CompletedJob cj;
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


void waitForJob(string jobType, int id) {
	if (jobType.compare("PARALLEL_MINIMAX") == 0) {
		vector<Job> jobsToWork;
		vector<Board> boards;
		vector<CompletedJob> completedJobs;

		slaveReceiveJobs(&jobsToWork, &boards);
		
	    printf("Process %d received jobs from process 0:\n", id);
	    for (int i = 0; i < jobsToWork.size(); i++) {
	    	//printJob(jobsToWork[i], world_rank);
	    	Job currentJob = jobsToWork[i];
	    	//Board currentBoard = currentJob.board;
	    	//int* data = currentJob.data;
	    	printf("Value extracted = %d\n", currentJob.id);//data[0]);
	    	//currentBoard->printBoard(BLACK);
	    }
	    /*
	    // Work on problems
	    for (int i = 0; i < jobsToWork.size(); i++) {
  			Job currentJob = jobsToWork[i];
  			while (!isJobFinished(currentJob)) {
  				executeJob(&currentJob);
  			}
  			CompletedJob cj = {currentJob.number};
  			completedJobs.push_back(cj);
  			printf("Process %d finished Job with Parent %d. Result: %d\n", 
  				world_rank, currentJob.parentNumber, currentJob.number);
  		}
		
	    slaveSendCompletedJobs(&completedJobs);*/
	}
}

void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs) {
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
			Board currentBoard = (*boards)[k];
			
			for (int w = 0; w < width; w++) {
				for (int h = 0; h < height; h++) {
					int num = currentBoard.getDisk(w, h);
					MPI_Send((void*)&num, 1, MPI_INT, i, w * width + h, MPI_COMM_WORLD);
				}
			}
		}
	}
}

void slaveReceiveJobs(vector<Job>* jobs, vector<Board>* boards) {
	// Probe for new incoming walkers
	MPI_Status status;
	MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

	// Resize your incoming walker buffer based on how much data is being received
	int incoming_size;
	MPI_Get_count(&status, MPI_BYTE, &incoming_size);
	jobs->resize(incoming_size / sizeof(Job));

	// Receive configuration information of Jobs
	MPI_Recv((void*)jobs->data(), incoming_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, 
		MPI_STATUS_IGNORE);

	// Receive array data of Jobs
	int width = jobs->front().width;
	int height = jobs->front().height;
	int data[width][height];
	
	for (int k = 0; k < jobs->size(); k++) {
		for (int w = 0; w < width; w++) {
			for (int h = 0; h < height; h++) {
				MPI_Recv(&data[w][h], 1, MPI_INT, 0, w * width + h, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
	}
	Board newBoard = Board(width, height);
	for (int w = 0; w < width; w++) {
		for (int h = 0; h < height; h++) {
			newBoard.setDisk(data[w][h], w, h);
		}
	}
	newBoard.printBoard(BLACK);
}

void slaveSendCompletedJobs(vector<CompletedJob>* jobs) {
	// Send back to Processor 0 for results to be merged
	MPI_Send((void*)jobs->data(), jobs->size() * sizeof(Job), 
		MPI_BYTE, 0, 0, MPI_COMM_WORLD);
	jobs->clear();
}

void masterReceiveCompletedJobs(vector<CompletedJob>* jobs, int numProcs) {
	for (int i = 1; i < numProcs; i++) {
		vector<CompletedJob> incomingCompletedJobs;

		// Probe for new incoming walkers
		MPI_Status status;
		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		// Resize your incoming walker buffer based on how much data is being received
		int incoming_size;
		MPI_Get_count(&status, MPI_BYTE, &incoming_size);
		incomingCompletedJobs.resize(incoming_size / sizeof(Job));

		MPI_Recv((void*)incomingCompletedJobs.data(), incoming_size, MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, 
			MPI_STATUS_IGNORE);
		// Add to completed jobs
		for (int j = 0; j < incomingCompletedJobs.size(); j++) {
			CompletedJob currentJob = incomingCompletedJobs[j];
			jobs->push_back(currentJob);
			//printf("TESTING:\n");
			//printJob(currentJob, world_rank);
			//currentJob.test.execute();
		}
		incomingCompletedJobs.clear();
	}
}
