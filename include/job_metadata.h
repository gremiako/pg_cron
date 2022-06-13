/*-------------------------------------------------------------------------
 *
 * job_metadata.h
 *	  definition of job metadata functions
 *
 * Copyright (c) 2010-2015, Citus Data, Inc.
 *
 *-------------------------------------------------------------------------
 */

#ifndef JOB_METADATA_H
#define JOB_METADATA_H


#include "nodes/pg_list.h"
#if (PG_VERSION_NUM < 120000)
#include "datatype/timestamp.h"
#endif

typedef enum
{
	CRON_STATUS_STARTING,
	CRON_STATUS_RUNNING,
	CRON_STATUS_SENDING,
	CRON_STATUS_CONNECTING,
	CRON_STATUS_SUCCEEDED,
	CRON_STATUS_FAILED
} CronStatus;

/* job metadata data structure */
typedef struct CronJob
{
	int64 jobId;
	char *scheduleText;
	entry schedule;
	char *command;
	char *nodeName;
	int nodePort;
	char *database;
	char *userName;
	bool active;
	Name jobName;
} CronJob;

#define JOBS_TABLE_NAME "job"
#define LT_JOB_EXT "lt_job_ext"

#define MODE_SINGLE 	"single"
#define MODE_TIMING 	"timing"
#define MODE_ASAP 		"asap"
#define MODE_NEXT 		"next"
#define MODE_FIXED 		"fixed"

#define COMMAND_SQL		"sql"
#define COMMAND_LINUX	"linux"

#define DEFAULT_FILED_LEN	16
#define MAX_STRING_LEN		1024

/* global settings */
extern char *CronHost;
extern bool CronJobCacheValid;


/* functions for retrieving job metadata */
extern void InitializeJobMetadataCache(void);
extern void ResetJobMetadataCache(void);
extern List * LoadCronJobList(void);
extern CronJob * GetCronJob(int64 jobId);

extern void InsertJobRunDetail(int64 runId, int64 *jobId, char *database, char *username, char *command, char *status);
extern void UpdateJobRunDetail(int64 runId, int32 *job_pid, char *status, char *return_message, TimestampTz *start_time,
									TimestampTz *end_time);
extern void queryCommandFromJobRunDetail(int64 runid, char *value, unsigned int insize);

extern int64 NextRunId(void);
extern void MarkPendingRunsAsFailed(void);
extern char *GetCronStatus(CronStatus cronstatus);

extern void queryFiledFromCron(int64 jobid, char *tablename, char *filed, char *value, unsigned int insize);
extern void updateCronActive(int64 jobid, char *active);
extern void keepDataFromCronRun(void);


#endif
