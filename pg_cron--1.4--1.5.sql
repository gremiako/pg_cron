/* pg_cron--1.4--1.5.sql */
ALTER TABLE cron.lt_job_ext ADD COLUMN commandtype text;

CREATE FUNCTION cron.schedule(job_name name, schedule text, command text, single_mode text, tmzone text, command_type text)
    RETURNS bigint
    LANGUAGE C STRICT
    AS 'MODULE_PATHNAME', $$cron_schedule_named_mode_zone_cmdtype$$;
COMMENT ON FUNCTION cron.schedule(name,text,text,text,text,text)
    IS 'schedule a pg_cron job';
