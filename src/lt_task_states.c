/*-------------------------------------------------------------------------
 *
 * src/lt_task_states.c
 *
 * Logic for storing and manipulating cron fixed interval mode task states.
 *
 * lightdb add 2022/3/26 for S202203046035
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"

#include "cron.h"
#include "pg_cron.h"
#include "task_states.h"

#include "access/hash.h"
#include "utils/hsearch.h"
#include "utils/memutils.h"


/* forward declarations */
static HTAB * CreateCronFixedTaskHash(void);
static CronFixedTask * GetCronFixedTask(int64 runId, CronTask *task);

/* global variables */
static MemoryContext CronFixedTaskContext = NULL;
static HTAB *CronFixedTaskHash = NULL;


/*
 * InitializeFixedTaskStateHash initializes the hash for storing fixed interval mode task states.
 */
void
InitializeFixedTaskStateHash(void)
{
	CronFixedTaskContext = AllocSetContextCreate(CurrentMemoryContext,
											"pg_cron fixed task context",
											ALLOCSET_DEFAULT_MINSIZE,
											(ALLOCSET_DEFAULT_INITSIZE * 8),
											ALLOCSET_DEFAULT_MAXSIZE);

	CronFixedTaskHash = CreateCronFixedTaskHash();
}

/*
 * CreateCronFixedTaskHash creates the hash for storing cron fixed interval mode task states.
 */
static HTAB *
CreateCronFixedTaskHash(void)
{
	HTAB *taskHash = NULL;
	HASHCTL info;
	int hashFlags = 0;

	memset(&info, 0, sizeof(info));
	info.keysize = sizeof(int64);
	info.entrysize = sizeof(CronFixedTask);
	info.hash = tag_hash;
	info.hcxt = CronFixedTaskContext;
	hashFlags = (HASH_ELEM | HASH_FUNCTION | HASH_CONTEXT);

	taskHash = hash_create("pg_cron fixed interval tasks", (32 * 16), &info, hashFlags);

	return taskHash;
}

/*
 * RefreshFixedTaskHash reloads the cron fixed tasks from the cron task list.
 */
void
RefreshFixedTaskHash(CronTask *task)
{
	int64 runId = 0;

	if (NULL == task)
	{
		elog(INFO, "RefreshFixedTaskHash task parameter is NULL");
		return;
	}

	runId = NextRunId();
	GetCronFixedTask(runId, task);
}

/*
 * GetCronFixedTask gets the current fixed interval mode task with the given run ID.
 */
static CronFixedTask *
GetCronFixedTask(int64 runId, CronTask *task)
{
	CronFixedTask *_curTask = NULL;
	int64 hashKey = runId;
	bool isPresent = false;
	int offsetlen = 0;

	_curTask = hash_search(CronFixedTaskHash, &hashKey, HASH_ENTER, &isPresent);
	if (_curTask && !isPresent)
	{
		if (NULL == task)
		{
			elog(INFO, "GetCronFixedTask task parameter is NULL");
			return NULL;
		}
		
		offsetlen = sizeof(task->jobId) + sizeof(task->runId);
		task->runId = runId;
		_curTask->runId = task->runId;
		_curTask->jobId = task->jobId;
		memcpy((char*)_curTask + offsetlen, (char*)task + offsetlen, sizeof(CronTask) - offsetlen);
	}

	return _curTask;
}

/*
 * CurrentFixedTaskList extracts the current list of tasks from the
 * cron fixed interval task hash.
 */
List *
CurrentFixedTaskList(void)
{
	List *taskList = NIL;
	CronTask *task = NULL;
	HASH_SEQ_STATUS status;

	hash_seq_init(&status, CronFixedTaskHash);

	while ((task = hash_seq_search(&status)) != NULL)
	{
		taskList = lappend(taskList, task);
	}

	return taskList;
}

/*
 * RemoveFixedTask remove the task for the given run ID.
 */
void
RemoveFixedTask(int64 runId)
{
	bool isPresent = false;

	hash_search(CronFixedTaskHash, &runId, HASH_REMOVE, &isPresent);
}
