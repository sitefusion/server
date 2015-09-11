
CREATE TABLE IF NOT EXISTS "processes" (
	"id" TEXT NOT NULL PRIMARY KEY,
	"type" TEXT NOT NULL,
	"ident" TEXT NOT NULL,
	"user" TEXT NOT NULL,
	"app" TEXT NOT NULL,
	"args" TEXT NOT NULL,
	"client_ip" TEXT NOT NULL,
	"port" INT4 NOT NULL,
	"pid" INT4 NOT NULL,
	"memory" INT8,
	"memory_peak" INT8,
	"nodes" INT4
);

CREATE TABLE IF NOT EXISTS "jobs" (
	"id" SERIAL NOT NULL PRIMARY KEY,
	"owner" TEXT NOT NULL,
	"job" TEXT NOT NULL,
	"args" TEXT NOT NULL,
	"minute" INT4 NOT NULL,
	"hour" INT4 NOT NULL,
	"monthday" INT4 NOT NULL,
	"month" INT4 NOT NULL,
	"weekday" INT4 NOT NULL,
	"mail_output" TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS "services" (
	"id" SERIAL NOT NULL PRIMARY KEY,
	"owner" TEXT NOT NULL,
	"service" TEXT NOT NULL,
	"args" TEXT NOT NULL,
	"daemon" INT2 NOT NULL
);
