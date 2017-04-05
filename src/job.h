
#include <mpi.h>
#include <deque>
#include <vector>
#include <math.h>
#include "config.h"
#include "board.h"
#include "point.h"

#ifndef JOB_H
#define JOB_H

using namespace std;

typedef struct {
	int id;
	int parentId;

	// Configurations
	int width;
	int height;

	// State of Job
	int player;
	int depthLeft;
	//Board board;
	int** data;
} Job;

typedef struct {
	int id;
	int parentId;

	// Useful values
	vector<point> points;
	vector<int> minimaxScores;
} CompletedJob;

// Job-specific functions
unsigned long getSizeOfJob(int width, int height);
vector<Job> splitJob(Job job);
CompletedJob executeJob(Job* job);
vector<CompletedJob> executeAllJobs(vector<Job>* job);

// Communications
void waitForJob(string jobType, int id);
void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs);
void slaveReceiveJobs(vector<Job>* jobs, vector<Board>* boards);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(vector<CompletedJob>* jobs, int numProcs);

#endif