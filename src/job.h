
#ifndef JOB_H
#define JOB_H

#include <mpi.h>
#include <deque>
#include <vector>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
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
	int player;
	int moveValue;
	int boardsAssessed;

} CompletedJob;

// Timing purposes
long long wallClockTime();

// Job-specific functions
CompletedJob executeJob(Job* job);
vector<CompletedJob> executeAllJobs(vector<Job>* job);

// Communications
void masterInitialiseJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs, 
	vector<point> validMoves, Board board, int player, int depth, int maxBoards, int cornerValue, int edgeValue);
void splitJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs,
	int numProcs, int jobsPerProc);
void slaveWaitForJob(string jobType, int id);
void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs);
void slaveReceiveJobs(vector<Job>* jobs);
void masterWorkOnJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(deque<CompletedJob>* jobs, int numProcs);
void masterRewindMinimaxStack(deque<CompletedJob>* jobs);

#endif