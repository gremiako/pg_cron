# src/test/modules/pg_cron/Makefile

EXTENSION = pg_cron
EXTVERSION = 1.4

DATA_built = $(EXTENSION)--1.0.sql
DATA = $(wildcard $(EXTENSION)--*--*.sql)
REGRESS = pg_cron-test 

# compilation configuration
MODULE_big = $(EXTENSION)
OBJS = $(patsubst %.c,%.o,$(wildcard src/*.c))
ifeq ($(CC),gcc)
    PG_CPPFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-maybe-uninitialized -Wno-implicit-fallthrough -Iinclude -I$(libpq_srcdir)
else
    PG_CPPFLAGS = -std=c99 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -Iinclude -I$(libpq_srcdir)
endif
SHLIB_LINK = $(libpq)
EXTRA_CLEAN += $(addprefix src/,*.gcno *.gcda) # clean up after profiling runs

ifdef USE_PGXS
PG_CONFIG = lt_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_cron
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

$(EXTENSION)--1.0.sql: $(EXTENSION).sql
	cat $^ > $@
