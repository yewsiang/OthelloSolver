
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

#define SLAVE_WANTS_JOBS 0
#define SLAVE_SENDING_JOBS 1
#define MASTER_SENDING_JOBS 2
#define MASTER_NO_JOBS 3

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
CompletedJob executeMinimaxJob(Job* job);
CompletedJob executeAlphaBetaJob(Job* job);
vector<CompletedJob> executeAllJobs(string algorithm, vector<Job>* job);

// Communications
void masterInitialiseJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs, 
	vector<point> validMoves, Board board, int player, int depth, int maxBoards, int cornerValue, int edgeValue);
void splitJobs(deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs,
	int numProcs, int jobsPerProc);

void slaveWaitForJob(string algorithm, int id);
void masterSendBatchJobs(deque<Job>* jobs, deque<Board>* boards, int numProcs, string jobDistribution);
void masterSendJobs(deque<Job>* jobs, deque<Board>* boards, int id, int numJobs, string jobDistribution);
void slaveReceiveJobs(vector<Job>* jobs);
void masterWorkOnJobs(string algorithm, deque<Job>* jobs, deque<Board>* boards, deque<CompletedJob>* waitingJobs);
void slaveSendCompletedJobs(vector<CompletedJob>* jobs);
void masterReceiveCompletedJobs(deque<CompletedJob>* jobs, int numProcs);
void masterReceiveCompletedJobsFromSlave(deque<CompletedJob>* jobs, int id);

void slaveRequestJob(string algorithm, int id);

void masterRewindMinimaxStack(deque<CompletedJob>* jobs);

#endif