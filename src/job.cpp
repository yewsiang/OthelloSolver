
#include "job.h"

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
		vector<CompletedJob> completedJobs;

		slaveReceiveJobs(&jobsToWork);
		
	    printf("Process %d received jobs from process 0:\n", id);
	    for (int i = 0; i < jobsToWork.size(); i++) {
	    	//printJob(jobsToWork[i], world_rank);
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

void masterSendJobs(deque<Job>* jobs, int numProcs) {
	int numJobs = jobs->size();
	for (int i = 1; i < numProcs; i++) {
		int problemSize = floor(numJobs * (i + 1) / numProcs) - floor(numJobs * i / numProcs);
		printf("For Processor %d, Problem size: %d\n", i, problemSize);

		vector<Job> jobsToSend;
		for (int j = 0; j < problemSize; j++) {
			Job currentJob = jobs->front();
			jobs->pop_front();
			jobsToSend.push_back(currentJob);
		}
		
		MPI_Send((void*)jobsToSend.data(), jobsToSend.size() * sizeof(Job), 
			MPI_BYTE, i, 0, MPI_COMM_WORLD);
	}
}

void slaveReceiveJobs(vector<Job>* jobs) {
	// Probe for new incoming walkers
	MPI_Status status;
	MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

	// Resize your incoming walker buffer based on how much data is being received
	int incoming_size;
	MPI_Get_count(&status, MPI_BYTE, &incoming_size);
	jobs->resize(incoming_size / sizeof(Job));

	MPI_Recv((void*)jobs->data(), incoming_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, 
		MPI_STATUS_IGNORE);
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
