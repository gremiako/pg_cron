### What is pg_cron?

pg_cron is a simple cron-based job scheduler for LightDB that runs inside the database as an extension. It allows you to schedule LightDB commands directly from the database:

```
-- Vacuum on Saturday at 3:30am (East eight time zone)
SELECT cron.schedule('30 3 * * 6', 'VACUUM');
 schedule
----------
       42

-- Vacuum every day at 10:00am (East eight time zone)
SELECT cron.schedule('nightly-vacuum', '0 10 * * *', 'VACUUM');
 schedule
----------
       43

-- Change to vacuum at 3:00am (East eight time zone)
SELECT cron.schedule('nightly-vacuum', '0 3 * * *', 'VACUUM');
 schedule
----------
       43

-- Stop scheduling jobs
SELECT cron.unschedule('nightly-vacuum' );
 unschedule
------------
          t

SELECT cron.unschedule(42);
 unschedule
------------
          t
        
```

pg_cron can support second-level precision:

```
-- Vacuum every day at 10:00:30am (East eight time zone)
SELECT cron.schedule('30 0 10 * * *', 'VACUUM');
 schedule
----------
       45

-- Vacuum every second
SELECT cron.schedule('dayly-vacuum', '* * * * * *', 'VACUUM');
 schedule
----------
       46

-- Change to Vacuum every 10 seconds
SELECT cron.schedule('dayly-vacuum', '*/10 * * * * *', 'VACUUM');
 schedule
----------
       46
        
```

pg_cron can support four task modes, include one-time tasks, asap takes, next interval tasks and fixed interval tasks. You can pass in the task mode in the fourth parameter and there are four parameters to choose from (If you want to configure the task mode, the first parameter task name must be passed in):

- `'single'` represents a one-time task, this means that when the task is executed for the first time, the task will not be executed again.

  ```
  -- Change to Vacuum only once immediately
  SELECT cron.schedule('dayly-vacuum', '* * * * * *', 'VACUUM', 'single');
   schedule
  ----------
         46
  
  -- Change to Vacuum every 30 seconds
  SELECT cron.schedule('dayly-vacuum', '*/30 * * * * *', 'VACUUM', 'next');
   schedule
  ----------
         46
  
  -- Change to Vacuum only once at the next 10:00:30am (East eight time zone)
  SELECT cron.schedule('dayly-vacuum', '30 0 10 * * *', 'VACUUM', 'single');
   schedule
  ----------
         46
  
  -- Change to Vacuum every day at 10:00:30am (East eight time zone)
  SELECT cron.schedule('dayly-vacuum', '30 0 10 * * *', 'VACUUM', 'next');
   schedule
  ----------
         46
                  
  ```

- `'asap'` represents a asap scheduled task, for the same task it runs at most one instance of a job at a time. If a second run is supposed to start before the first one finishes, then the second run is queued and started as soon as the first run completes.

  ```
  -- Change to Vacuum every 30 seconds
  SELECT cron.schedule('dayly-vacuum', '*/30 * * * * *', 'VACUUM', 'asap');
   schedule
  ----------
         46
  
  -- Change to Vacuum every day at 10:00:30am (East eight time zone)
  SELECT cron.schedule('dayly-vacuum', '30 0 10 * * *', 'VACUUM', 'asap');
   schedule
  ----------
         46
                  
  ```

- `'next'` represents a next interval scheduled task, for the same task it runs at most one instance of a job at a time. If a second run is supposed to start before the first one finishes, then the second run is queued and started at the time point of the next timing cycle.

  Compatible with the previous version, the mode parameter input `'timing'` is the same as `'next'`.

  ```
  -- Change to Vacuum every 30 seconds
  SELECT cron.schedule('dayly-vacuum', '*/30 * * * * *', 'VACUUM', 'next');
   schedule
  ----------
         46
  
  -- Change to Vacuum every day at 10:00:30am (East eight time zone)
  SELECT cron.schedule('dayly-vacuum', '30 0 10 * * *', 'VACUUM', 'next');
   schedule
  ----------
         46
                  
  ```

- `'fixed'` represents a fixed interval scheduled task, for the same task it runs at most four instances of a job at a time by default. If a second run is supposed to start before the first one finishes, then the second run will not wait, and will start at the time point of the timing cycle, then will be executed in parallel with the first unfinished task.

  You can modify the maximum number of concurrent executions for the same task when it expires by configuring the `'cron.max_connections_per_task'` GUC parameters in the postgresql.conf and restarting the database to take effect. The maximum upper limit is 16.

  ```
  -- Change to Vacuum every 30 seconds
  SELECT cron.schedule('dayly-vacuum', '*/30 * * * * *', 'VACUUM', 'fixed');
   schedule
  ----------
         46
  
  -- Change to Vacuum every day at 10:00:30am (East eight time zone)
  SELECT cron.schedule('dayly-vacuum', '30 0 10 * * *', 'VACUUM', 'fixed');
   schedule
  ----------
         46
                  
  ```

pg_cron can support time zone configuration. You can pass the timezone value in the fifth parameter. If you want to configure the time zone, the first parameter task name and the fourth parameter task mode must be passed in. If no time zone is configured, the default is East eight time zone:

```
-- Change to Vacuum every day at 10:00am (GMT)
SELECT cron.schedule('dayly-vacuum', '0 10 * * *', 'VACUUM', 'next', '0');
 schedule
----------
       46

-- Change to vacuum every day at 10:00am (West ten time zone)
SELECT cron.schedule('dayly-vacuum', '0 10 * * *', 'VACUUM', 'next', '-10');
 schedule
----------
       46

-- Change to vacuum only once at the next 10:00am (East six time zone)
SELECT cron.schedule('dayly-vacuum', '0 10 * * *', 'VACUUM', 'single', '6');
 schedule
----------
       46
        
```

pg_cron can run multiple jobs in parallel, and by default it uses next interval mode, i.e. it runs at most one instance of a job at a time. If a second run is supposed to start before the first one finishes, then the second run is queued and started at the time point of the next timing cycle.

pg_cron supports a 30-second timeout for scheduled tasks by default, which is valid for all types of tasks. If the task execution times out, it will log the error in `cron.job_run_details` and return, waiting for the next execution. You can modify the timing task timeout time by setting the guc parameter `cron.task_running_timeout` in postgresql.conf and restarting the database to take effect. The maximum value is 1800 seconds; if it is set to 0, it means there is no timeout limit.

The schedule uses the standard cron syntax, in which * means "run every time period", and a specific number means "but only at this time":

```
┌───────────── min (0 - 59)
│ ┌────────────── hour (0 - 23)
│ │ ┌─────────────── day of month (1 - 31)
│ │ │ ┌──────────────── month (1 - 12)
│ │ │ │ ┌───────────────── day of week (0 - 6) (0 to 6 are Sunday to
│ │ │ │ │                  Saturday, or use names; 7 is also Sunday)
│ │ │ │ │
│ │ │ │ │
* * * * *
        
```

An easy way to create a cron schedule is: [crontab.guru](https://crontab.guru/).

It has been enhanced on the basis of standard cron syntax to supports second-level tasks:

```
┌─────────────second (0 - 59)
│ ┌───────────── minute (0 - 59)
│ │ ┌────────────── hour (0 - 23)
│ │ │ ┌─────────────── day of month (1 - 31)
│ │ │ │ ┌──────────────── month (1 - 12)
│ │ │ │ │ ┌───────────────── day of week (0 - 6) (0 to 6 are Sunday to
│ │ │ │ │ │                  Saturday, or use names; 7 is also Sunday)
│ │ │ │ │ │
│ │ │ │ │ │
* * * * * *
        
```

For security, jobs are executed in the database in which the cron.schedule function is called with the same permissions as the current user. In addition, users are only able to see their own jobs in the `cron.job` table and `cron.lt_job` view.
