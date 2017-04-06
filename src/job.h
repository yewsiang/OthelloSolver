
#ifndef JOB_H
#define JOB_H

#include <mpi.h>
#include <deque>
#include <vector>
#include <math.h>
#include "config.h"
#include "board.h"
#include "point.h"
#include "solver.h"

using namespace std;

typedef struct {
	// To identify the original move ID
	int moveId;

	// Configurations
	int width;
	int height;
	int maxBoards;
	int cornerValue;
	int edgeValue;

	// State of Job
	int player;
	int depthLeft;
	Board* board;
} Job;

typedef struct {
	// To identify the original move ID
	int moveId;

	// Computed values
	int moveValue;
	int boardsAssessed;

} CompletedJob;

// Job-specific functions
void splitJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs, int jobsPerProc);
CompletedJob executeJob(Job* job);
vector<CompletedJob> executeAllJobs(vector<Job>* job);

// Communications
void waitForJob(string jobType, int id);
void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs);
void slaveReceiveJobs(vector<Job>* jobs);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(vector<CompletedJob>* jobs, int numProcs);

#endif