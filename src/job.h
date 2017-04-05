
#include <mpi.h>
#include <deque>
#include <vector>
#include <math.h>
#include "board.h"
#include "point.h"

#ifndef JOB_H
#define JOB_H

using namespace std;

typedef struct {
	int id;
	int parentId;

	// State of Job
	//Board* board;
	int player;
	int depthLeft;
} Job;

typedef struct {
	int id;
	int parentId;

	// Useful values
	vector<point> points;
	vector<int> minimaxScores;
} CompletedJob;

// Job-specific functions
vector<Job> splitJob(Job job);
CompletedJob executeJob(Job* job);
vector<CompletedJob> executeAllJobs(vector<Job>* job);

// Communications
void waitForJob(string jobType, int id);
void masterSendJobs(deque<Job>* jobs, int numProcs);
void slaveReceiveJobs(vector<Job>* jobs);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(vector<CompletedJob>* jobs, int numProcs);

#endif