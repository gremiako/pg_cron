/*-------------------------------------------------------------------------
 *
 * task_states.h
 *	  definition of task state functions
 *
 * Copyright (c) 2010-2015, Citus Data, Inc.
 *
 *-------------------------------------------------------------------------
 */

#ifndef TASK_STATES_H
#define TASK_STATES_H


#include "job_metadata.h"
#include "libpq-fe.h"
#include "postmaster/bgworker.h"
#include "storage/dsm.h"
#include "utils/timestamp.h"


typedef enum
{
	CRON_TASK_WAITING = 0,
	CRON_TASK_START = 1,
	CRON_TASK_CONNECTING = 2,
	CRON_TASK_SENDING = 3,
	CRON_TASK_RUNNING = 4,
	CRON_TASK_RECEIVING = 5,
	CRON_TASK_DONE = 6,
	CRON_TASK_ERROR = 7,
	CRON_TASK_BGW_START = 8,
	CRON_TASK_BGW_RUNNING = 9
} CronTaskState;

typedef enum
{
	CRON_MODE_NEXT = 0,
	CRON_MODE_ASAP = 1,
	CRON_MODE_FIXED = 2,
	CRON_MODE_SINGLE = 3
} CronModeState;

typedef enum
{
	CRON_COMMAND_TYPE_SQL = 0,
	CRON_COMMAND_TYPE_LINUX = 1,
} CronCommandType;

struct BackgroundWorkerHandle
{
	int slot;
	uint64 generation;
};

typedef struct CronTask
{
	int64 jobId;
	int64 runId;
	CronTaskState state;
	uint pendingRunCount;
	PGconn *connection;
	PostgresPollingStatusType pollingStatus;
	TimestampTz startDeadline;
	bool isSocketReady;
	bool isActive;
	char *errorMessage;
	bool freeErrorMessage;
	dsm_segment *seg;
	BackgroundWorkerHandle handle;
	int mode;
	int commandtype;
} CronTask;

typedef struct CronFixedTask
{
	int64 runId;
	int64 jobId;
	CronTaskState state;
	uint pendingRunCount;
	PGconn *connection;
	PostgresPollingStatusType pollingStatus;
	TimestampTz startDeadline;
	bool isSocketReady;
	bool isActive;
	char *errorMessage;
	bool freeErrorMessage;
	dsm_segment *seg;
	BackgroundWorkerHandle handle;
	int mode;
	int commandtype;
} CronFixedTask;

extern void InitializeTaskStateHash(void);
extern void RefreshTaskHash(void);
extern List * CurrentTaskList(void);
extern void InitializeCronTask(CronTask *task, int64 jobId);
extern void RemoveTask(int64 jobId);

extern void InitializeFixedTaskStateHash(void);
extern void RefreshFixedTaskHash(CronTask *task);
extern List * CurrentFixedTaskList(void);
extern void RemoveFixedTask(int64 runId);

#endif
