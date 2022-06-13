CREATE EXTENSION pg_cron VERSION '1.0';
SELECT extversion FROM pg_extension WHERE extname='pg_cron';
ALTER EXTENSION pg_cron UPDATE TO '1.5';
SELECT extversion FROM pg_extension WHERE extname='pg_cron';

-- Vacuum every day at 10:00am (GMT)
SELECT cron.schedule('0 10 * * *', 'VACUUM');

-- Stop scheduling a job
SELECT cron.unschedule(1);


-- Invalid input: input too long
SELECT cron.schedule(repeat('a', 1000), '');

-- Try to update pg_cron on restart
SELECT cron.schedule('@restar', 'ALTER EXTENSION pg_cron UPDATE');
SELECT cron.schedule('@restart', 'ALTER EXTENSION pg_cron UPDATE');

-- Vacuum every day at 10:00am (GMT)
SELECT cron.schedule('myvacuum', '0 10 * * *', 'VACUUM');

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

-- Make that 11:00am (GMT)
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM');

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

-- Make that VACUUM FULL
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM FULL');

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

-- Stop scheduling a job
SELECT cron.unschedule('myvacuum');

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

-- Make that 11:00:12am (GMT)
SELECT cron.schedule('myvacuum', '12 0 11 * * *', 'VACUUM');

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

-- Stop scheduling a job
SELECT cron.unschedule(6);

-- Make that 11:00am (GMT) single
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'single');

SELECT jobid, jobname, schedule, command, mode FROM cron.lt_job ORDER BY jobid;

-- Make that 11:00am (GMT) timing
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'timing');

SELECT jobid, jobname, schedule, command, mode FROM cron.lt_job ORDER BY jobid;

-- Stop scheduling a job
SELECT cron.unschedule(7);

-- Make that 11:00am (GMT) zone 8
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'timing', '8');

SELECT jobid, jobname, schedule, command, timezone FROM cron.lt_job ORDER BY jobid;

-- Make that 11:00am (GMT) zone -8
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'timing', '-8');

SELECT jobid, jobname, schedule, command, timezone FROM cron.lt_job ORDER BY jobid;

-- Make that 11:00am (GMT) next interval
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'next');

SELECT jobid, jobname, schedule, command, mode FROM cron.lt_job ORDER BY jobid;

-- Make that 11:00am (GMT) asap
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'asap');

SELECT jobid, jobname, schedule, command, mode FROM cron.lt_job ORDER BY jobid;

-- Make that 11:00am (GMT) fixed interval
SELECT cron.schedule('myvacuum', '0 11 * * *', 'VACUUM', 'fixed');

SELECT jobid, jobname, schedule, command, mode FROM cron.lt_job ORDER BY jobid;

-- Stop scheduling a job
SELECT cron.unschedule(9);

SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;

SELECT cron.schedule('test1', '* * * * * *', 'select1', 'next', '8', 'sql');
SELECT cron.schedule('test2', '* * * * * *', 'select2', 'asap', '8', 'sql');
SELECT cron.schedule('test3', '* * * * * *', 'select3', 'single', '8', 'sql');
SELECT cron.schedule('test4', '* * * * * *', 'select4', 'fixed', '8', 'sql');
SELECT cron.schedule('test5', '* * * * * *', 'echo12345', 'next', '8', 'linux');
SELECT jobid, jobname, schedule, command FROM cron.job ORDER BY jobid;
SELECT pg_sleep(3);

DROP EXTENSION pg_cron;
