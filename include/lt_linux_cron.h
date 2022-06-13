/*-------------------------------------------------------------------------
 *
 * lt_linux_cron.h
 * definition of linux cron task functions
 *
 * lightdb add 2022/4/12 for S202204117426
 *
 *-------------------------------------------------------------------------
 */
#ifndef LT_LINUX_CRON_H
#define LT_LINUX_CRON_H

#include "job_metadata.h"
#include "libpq-fe.h"
#include "postmaster/bgworker.h"
#include "storage/dsm.h"
#include "storage/ipc.h"
#include "utils/timestamp.h"


extern void ManageLinuxCronTask(CronTask *task);


#endif
