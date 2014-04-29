pgaudit
=======

This is the initial version of an auditing module for Postgres.

It collects audit events from various sources and logs them in CSV
format including a timestamp, user information, details of objects
affected (if any), and the fully-qualified command text (whenever
available).

All DDL, DML (including SELECT), and utility commands are supported.
These are categorised as described below, and audit logging for each
group of commands may be enabled or disabled by the superuser. Once
enabled, however, audit logging may not be disabled by a user.

What about log_statement = 'all'?
---------------------------------

1. pgaudit logs fully-qualified names

A query like «delete from x» in the log file can be interpreted only
with reference to the current search_path setting. In contrast, this
module always logs fully-qualified object names, e.g. "public.x".

For DDL commands that have appropriate event trigger support, we log an
unambigous representation of the command text, not just the query string
as supplied by the user.

2. pgaudit creates a log entry for each affected object

A query that refers to multiple objects results in a log entry for each
object involved, so the effects of «select * from some_view» can be seen
rather than inferred. Searching for all accesses to a particular table
is also straightforward.

3. pgaudit provides finer-grained control over what events are logged

With log_statement, one may select none, ddl, mod, or all. With pgaudit,
individual groups of commands may be selected for logging. Want to log
only GRANT/REVOKE operations? You can.

Installation
------------

The latest pgaudit code is available at
https://github.com/2ndQuadrant/pgaudit

This module will work with Postgres 9.3 and 9.4 (not yet released at the
time of writing), but it needs updated event trigger code in order to
log a complete, unambiguous representation of DDL commands.

We hope that the necessary event trigger code will be available in 9.5,
but until then you will have to build your own Postgres to see pgaudit
at its best. The necessary code is available in the deparse branch of
git://git.postgresql.org/git/2ndquadrant_bdr.git

First, build and install Postgres as usual from the deparse branch. Copy
pgaudit into contrib/pgaudit and edit the Makefile to uncomment the line
that defines "USE_DEPARSE_FUNCTIONS". Then run "make install".

If you want to use it against an earlier version of Postgres, just run
"make USE_PGXS=1 install" in the pgaudit directory.

Once the module is installed, edit postgresql.conf and set:

	shared_preload_libraries = 'pgaudit'

Then start the server and run:

	CREATE EXTENSION pgaudit;

Configuration
-------------

Audit logging is controlled by the pgaudit.log configuration variable,
which may be set to a comma-separated list of tokens identifying what
classes of commands to log. For example,

	pgaudit.log = 'read, write, user'

pgaudit.log may be set to an empty string or "none" to disable logging,
or to any combination of the following logging classes:

	READ		SELECT commands
	WRITE		INSERT, UPDATE, DELETE, TRUNCATE
	PRIVILEGE	GRANT, REVOKE, etc.
	USER		CREATE/DROP/ALTER ROLE
	DEFINITION	DDL: CREATE/DROP/ALTER for tables, etc.
	CONFIG		CREATE OPERATOR, etc.
	ADMIN		VACUUM, REINDEX, ANALYSE, …
	FUNCTION	Non-catalog function execution

pgaudit.log may be set in postgresql.conf (to apply globally), or as a
per-database or per-user setting:

	ALTER DATABASE xxx SET pgaudit.log = '…'

or:

	ALTER ROLE xxx SET pgaudit.log = '…'

Log format
----------

We log audit events in CSV format with the following fields:

	[AUDIT],<timestamp>,<database>,<username>,<effective username>,
		<class>,<tag>,<object type>,<object id>,
		<command text>

*class* is the name of a logging class (READ, WRITE, etc.)

*tag* is the command tag (e.g. SELECT)

*object type* is the type of object affected, if any (e.g. TABLE)

*object id* is some way to identify the affected object, usually a
fully-qualified name

*command text* is the full text of the command.

Note that not all fields are always available.

Here are some examples of log output:

LOG:  [AUDIT],2014-04-28 12:21:01.15293+09,auditdb,dbusr,dbusr,DEFINITION,CREATE TABLE,table,public.x,CREATE  TABLE  public.x (a pg_catalog.int4   , b pg_catalog.text   COLLATE pg_catalog."default")   WITH (oids=OFF)
LOG:  [AUDIT],2014-04-28 12:26:11.82442+09,auditdb,dbusr,dbusr,WRITE,INSERT,TABLE,public.x,
LOG:  [AUDIT],2014-04-28 12:26:49.157767+09,auditdb,dbusr,dbusr,READ,SELECT,TABLE,public.x,
LOG:  [AUDIT],2014-04-28 12:27:14.26793+09,auditdb,dbusr,dbusr,DEFINITION,ALTER TABLE,table,public.x,ALTER TABLE public.x ADD COLUMN c pg_catalog.date
LOG:  [AUDIT],2014-04-28 12:28:37.035584+09,auditdb,dbusr,dbusr,WRITE,UPDATE,TABLE,public.x,
LOG:  [AUDIT],2014-04-28 12:28:48.378428+09,auditdb,dbusr,dbusr,ADMIN,VACUUM,,,VACUUM x ;

Design overview
---------------

We collect audit events from event triggers for any operations with
event trigger support. (For some commands, this also gives us the
unambiguous deparsed command representation.) Other DDL and utility
commands are collected by a utility hook, and DML and SELECT events
are collected by an executor hook.

See DESIGN for more details and future improvements.

Known problems
--------------

Statements are audit-logged even if the transaction they're in is later
rolled back. This is sometimes desirable (e.g. with SELECT), but makes
it more difficult to tell what happened.

Some utility statements are audit-logged even though they subsequently
fail (e.g. «set shared_buffers = '32MB'»).

Deparsed query text is only available for CREATE, not DROP.

'ALTER TABLE … DROP …' is logged twice (as a CREATE, then a DROP).

Some bugs of varying severity in the deparse code have been reported
upstream. Some have been fixed already, but the code is under active
development, and other bugs still await fixes.

Bug reports and other feedback are welcome.

Authors
-------
Ian Barwick <ian@2ndQuadrant.com>
Abhijit Menon-Sen <ams@2ndQuadrant.com>

The research leading to these results has received funding from the
European Union's Seventh Framework Programme (FP7/2007-2013) under
grant agreement n° 318633. http://axleproject.eu
