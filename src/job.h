
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
	// For combination of results
	int id;
	int parentId;

	// Configurations
	int width;
	int height;
	int maxBoards;
	int cornerValue;
	int edgeValue;

	// State of Job
	int player;
	int depthLeft;
	int boardsAssessed;
	Board* board;
} Job;

typedef struct {
	// For combination of results
	int id;
	int parentId;

	// Computed values
	int moveValue;
	int boardsAssessed;

} CompletedJob;

// Job-specific functions
void splitJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs,
	int numProcs, int jobsPerProc);
CompletedJob executeJob(Job* job);
vector<CompletedJob> executeAllJobs(vector<Job>* job);

// Communications
void waitForJob(string jobType, int id);
void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs);
void slaveReceiveJobs(vector<Job>* jobs);
void masterWorkOnJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(deque<CompletedJob>* jobs, int numProcs);

#endif