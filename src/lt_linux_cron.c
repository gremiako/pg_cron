/*-------------------------------------------------------------------------
 *
 * src/lt_linux_cron.c
 *
 * A task is what is support linux os commands
 *
 * lightdb add 2022/4/12 for S202204117426
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "cron.h"
#include "pg_cron.h"
#include "task_states.h"
#include "lt_linux_cron.h"
#include "postmaster/fork_process.h"
#include "storage/proc.h"

#include<unistd.h>


void RunLinuxTask(Datum arg);
static void cron_linux_background_worker_sigterm(SIGNAL_ARGS);
static bool system_shell_run(char *arg_str);


/*
 * ManageLinuxCronTask implements the linux cron task state machine.
 */
void
ManageLinuxCronTask(CronTask *task)
{
	if (NULL == task)
	{
		elog(ERROR, "ManageLinuxCronTask task is NULL");
	}
	
	if (CRON_TASK_START == task->state)
	{
		BackgroundWorker worker;
		BackgroundWorkerHandle *handle;
		pid_t pid = 0;
		BgwHandleStatus status;
		CronStatus cronstate = CRON_STATUS_RUNNING;
		TimestampTz start_time = GetCurrentTimestamp();
		char *retMsg = NULL;

		task->state = CRON_TASK_RUNNING;

		memset(&worker, 0, sizeof(worker));
		worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
		worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
		worker.bgw_restart_time = BGW_NEVER_RESTART;
		snprintf(worker.bgw_name, BGW_MAXLEN, "cron_linux");
		snprintf(worker.bgw_type, BGW_MAXLEN, "cron_linux");
		sprintf(worker.bgw_library_name, "pg_cron");
		sprintf(worker.bgw_function_name, "RunLinuxTask");
		worker.bgw_main_arg = Int64GetDatum(task->runId);
		/* set bgw_notify_pid so that we can use WaitForBackgroundWorkerStartup */
		worker.bgw_notify_pid = MyProcPid;

		if (!RegisterDynamicBackgroundWorker(&worker, &handle))
		{
			elog(LOG, "ManageLinuxCronTask RegisterDynamicBackgroundWorker failed");
			retMsg = "RegisterDynamicBackgroundWorker failed";
			task->state = CRON_TASK_ERROR;
		}

		task->handle = *handle;
		status = WaitForBackgroundWorkerStartup(&task->handle, &pid);
		if (status != BGWH_STARTED && status != BGWH_STOPPED)
		{
			elog(LOG, "WaitForBackgroundWorkerStartup failed, status %d", status);
			retMsg = "WaitForBackgroundWorkerStartup failed";
			task->state = CRON_TASK_ERROR;
		}

		
		if (CRON_TASK_ERROR == task->state)
			cronstate = CRON_STATUS_FAILED;

		if (CronLogRun)
			UpdateJobRunDetail(task->runId, &pid, GetCronStatus(cronstate), retMsg, &start_time, NULL);
	}
}

/*
 * Run pg_cron linux command.
 */
void
RunLinuxTask(Datum arg)
{
	char command[MAX_STRING_LEN] = {0};
	int64 runid = 0;
	TimestampTz end_time = 0;
	char *retMsg = NULL;
	CronStatus cronstate = CRON_STATUS_SUCCEEDED;

	pqsignal(SIGTERM, cron_linux_background_worker_sigterm);
	BackgroundWorkerUnblockSignals();

	/* Connect to our database */
#if (PG_VERSION_NUM < 110000)
	BackgroundWorkerInitializeConnection(CronTableDatabaseName, NULL);
#else
	BackgroundWorkerInitializeConnection(CronTableDatabaseName, NULL, 0);
#endif

	do
	{
		runid = DatumGetInt64(arg);
		if (!runid)
		{
			elog(LOG, "cron linux runid is empty");
			cronstate = CRON_STATUS_FAILED;
			retMsg = "cron linux runid is empty";
			break;
		}

		if (CronLogRun)
		{
			queryCommandFromJobRunDetail(runid, command, MAX_STRING_LEN);
		}
		else
		{
			elog(LOG, "cron.log_run is closed");
			cronstate = CRON_STATUS_FAILED;
			retMsg = "cron.log_run is closed";
			break;
		}

		if (0 == strlen(command))
		{
			elog(LOG, "cron linux command is empty");
			cronstate = CRON_STATUS_FAILED;
			retMsg = "cron linux command is empty";
			break;
		}

		if (!system_shell_run(command))
		{
			int save_errno = errno;
	
			cronstate = CRON_STATUS_FAILED;
			retMsg = strerror(save_errno);

			elog(LOG, "linux commond [%s] execution failed, errno[%d]", command, save_errno);
			break;
		}

		retMsg = "linux command execution complete";
		end_time = GetCurrentTimestamp();
	}while(0);
	
	if (CronLogRun)
		UpdateJobRunDetail(runid, NULL, GetCronStatus(cronstate), retMsg, NULL, &end_time);

	proc_exit(0);
}

/*
 * Signal handler for SIGTERM for background workers
 * 		When we receive a SIGTERM, we set InterruptPending and ProcDiePending
 * 		just like a normal backend.  The next CHECK_FOR_INTERRUPTS() will do the
 * 		right thing.
 */
static void
cron_linux_background_worker_sigterm(SIGNAL_ARGS)
{
	int save_errno = errno;

	if (MyProc)
		SetLatch(&MyProc->procLatch);

	if (!proc_exit_inprogress)
	{
		InterruptPending = true;
		ProcDiePending = true;
	}

	errno = save_errno;
}

static bool
system_shell_run(char *arg_str)
{
	pid_t status;

	if (!arg_str || 0 == strlen(arg_str))
		return false;

	status = system(arg_str);
	if(-1 == status)
	{
		return false;
	}
	else
	{
		if(WIFEXITED(status))
			return (0 == WEXITSTATUS(status));
		else
			return false;
	}
}
