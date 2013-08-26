#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <sqlite3.h>
#include <zscope.h>

struct zs_ctx {
	struct		sqlite3 *zc_conn;
	size_t		zc_lastfileid;
	size_t		zc_lastsymbolid;
};

#define ZSC_FILETABLE \
	"CREATE TABLE filetable (" \
	"id INTEGER, " \
	"file TEXT, " \
	"UNIQUE(file), " \
	"PRIMARY KEY(id));"
#define ZSS_FILETABLE \
	"SELECT id, file FROM filetable WHERE file = '%s';"
#define ZSI_FILETABLE \
	"INSERT INTO filetable (id, file) VALUES (%zd, '%s');"

#define ZSC_SYMBOLTABLE \
	"CREATE TABLE symboltable (" \
	"id INTEGER, " \
	"type INTEGER, " \
	"name TEXT, " \
	"UNIQUE(name), " \
	"PRIMARY KEY(id));"
#define ZSS_SYMBOLTABLE \
	"SELECT id, type, name FROM symboltable WHERE name = '%s';"
#define ZSI_SYMBOLTABLE \
	"INSERT INTO symboltable (id, type, name) VALUES (%zd, %d, '%s');"

#define ZSC_USAGETABLE \
	"CREATE TABLE usagetable (" \
	"file_id INTEGER, " \
	"symbol_id INTEGER, " \
	"line INTEGER, " \
	"is_definition INTEGER, " \
	"PRIMARY KEY(file_id, symbol_id, line));"
#define ZSS_USAGETABLE \
	"SELECT file_id, symbol_id, line, is_definition FROM usagetable WHERE symbol_id = %zd;"
#define ZSI_USAGETABLE \
	"INSERT INTO usagetable (file_id, symbol_id, line, is_definition) VALUES (%zd, %zd, %zd, %d);"

#define MAX_QUERY_SIZE	(256)
enum query_result {
	QR_ERROR,
	QR_TRYAGAIN,
	QR_RESULT,
	QR_END,
};

static enum query_result zs_do_query(struct zs_ctx *, const char *, size_t, struct sqlite3_stmt **);
static size_t zs_find_fileid(struct zs_ctx *, const char *);
static size_t zs_find_symbolid(struct zs_ctx *, const char *);
static size_t zs_get_fileid(struct zs_ctx *, const char *);
static size_t zs_get_symbolid(struct zs_ctx *, const char *, enum zitem_type);

/* This function does the SQLite query. */
static enum query_result
zs_do_query(struct zs_ctx *zc, const char *stmt, size_t stmtlen, struct sqlite3_stmt **ss)
{
	int ret;
	enum query_result qr;

	qr = QR_ERROR;
	if (*ss != NULL)
		goto skip_prepare;

	ret = sqlite3_prepare_v2(zc->zc_conn, stmt, stmtlen, ss, NULL);
	if (ret != SQLITE_OK) {
		EPRINTF("ERROR preparing query '%s': (%d) %s",
		    stmt, ret, sqlite3_errmsg(zc->zc_conn));
		return (QR_ERROR);
	}

skip_prepare:
	ret = sqlite3_step(*ss);
	switch (ret) {
	case SQLITE_BUSY:
		DPRINTF("Database is busy");
		return (QR_TRYAGAIN);
	case SQLITE_ROW:
		return (QR_RESULT);

	case SQLITE_DONE:
		qr = QR_END;
		break;

	case SQLITE_ERROR:
	case SQLITE_MISUSE:
	default:
		EPRINTF("Database error ocurred: (%d) %s",
		    ret, sqlite3_errmsg(zc->zc_conn));
		qr = QR_ERROR;
		break;
	}

	sqlite3_finalize(*ss);
	ss = NULL;
	return (qr);
}

static size_t
zs_find_fileid(struct zs_ctx *zc, const char *file)
{
	struct	sqlite3_stmt *ss;
	size_t	querylen, result;
	enum	query_result qr;
	char	query[MAX_QUERY_SIZE];

	ss = NULL;
	result = SIZE_MAX;
	querylen = snprintf(query, sizeof(query), ZSS_FILETABLE, file);
	while ((qr = zs_do_query(zc, query, querylen, &ss)) != QR_END) {
		switch (qr) {
		case QR_ERROR:
			EPRINTF("Failed to find file");
			return (SIZE_MAX);
		case QR_TRYAGAIN:
			break;
		case QR_RESULT:
			result = sqlite3_column_int(ss, 1);
			break;

		default:
			EPRINTF("Unexpected query result (%d)", qr);
			break;
		}
	}
	return (result);
}

static size_t
zs_get_fileid(struct zs_ctx *zc, const char *file)
{
	struct	sqlite3_stmt *ss;
	size_t	querylen, result;
	enum	query_result qr;
	char	query[MAX_QUERY_SIZE];

	result = zs_find_fileid(zc, file);
	if (result != SIZE_MAX)
		return (result);

	ss = NULL;
	result = SIZE_MAX;
	querylen = snprintf(query, sizeof(query), ZSI_FILETABLE,
	    zc->zc_lastfileid, file);
	while ((qr = zs_do_query(zc, query, querylen, &ss)) != QR_END) {
		switch (qr) {
		case QR_ERROR:
			EPRINTF("Failed to insert file");
			return (SIZE_MAX);
		case QR_TRYAGAIN:
			break;

		case QR_RESULT:
		default:
			EPRINTF("Unexpected query result (%d)", qr);
			break;
		}
	}
	return (zc->zc_lastfileid++);
}

static size_t
zs_find_symbolid(struct zs_ctx *zc, const char *symbol)
{
	struct	sqlite3_stmt *ss;
	size_t	querylen, result;
	enum	query_result qr;
	char	query[MAX_QUERY_SIZE];

	ss = NULL;
	result = SIZE_MAX;
	querylen = snprintf(query, sizeof(query), ZSS_SYMBOLTABLE, symbol);
	while ((qr = zs_do_query(zc, query, querylen, &ss)) != QR_END) {
		switch (qr) {
		case QR_ERROR:
			EPRINTF("Failed to find symbol");
			return (SIZE_MAX);
		case QR_TRYAGAIN:
			break;
		case QR_RESULT:
			result = sqlite3_column_int(ss, 1);
			break;

		default:
			EPRINTF("Unexpected query result (%d)", qr);
			break;
		}
	}
	return (result);
}

static size_t
zs_get_symbolid(struct zs_ctx *zc, const char *symbol, enum zitem_type type)
{
	struct	sqlite3_stmt *ss;
	size_t	querylen, result;
	enum	query_result qr;
	char	query[MAX_QUERY_SIZE];

	result = zs_find_symbolid(zc, symbol);
	if (result != SIZE_MAX)
		return (result);

	ss = NULL;
	result = SIZE_MAX;
	querylen = snprintf(query, sizeof(query), ZSI_SYMBOLTABLE,
	    zc->zc_lastsymbolid, type, symbol);
	while ((qr = zs_do_query(zc, query, querylen, &ss)) != QR_END) {
		switch (qr) {
		case QR_ERROR:
			EPRINTF("Failed to insert symbol");
			return (SIZE_MAX);
		case QR_TRYAGAIN:
			break;

		case QR_RESULT:
		default:
			EPRINTF("Unexpected query result (%d)", qr);
			break;
		}
	}
	return (zc->zc_lastsymbolid++);
}

int
zs_register_symbol(struct zs_ctx *zc, const char *file, size_t line,
    const char *symbol, enum zitem_type type, int is_definition)
{
	struct	sqlite3_stmt *ss;
	size_t	querylen;
	size_t	fileid, symbolid;
	enum	query_result qr;
	char	query[MAX_QUERY_SIZE];


	fileid = zs_get_fileid(zc, file);
	if (fileid == SIZE_MAX) {
		EPRINTF("Could not register file '%s'", file);
		return (-1);
	}

	symbolid = zs_get_symbolid(zc, symbol, type);
	if (symbolid == SIZE_MAX) {
		EPRINTF("Could not register symbol '%s'", file);
		return (-1);
	}

	ss = NULL;
	querylen = snprintf(query, sizeof(query), ZSI_USAGETABLE,
	   fileid, symbolid, line, is_definition);
	while ((qr = zs_do_query(zc, query, querylen, &ss)) != QR_END) {
		switch (qr) {
		case QR_ERROR:
			EPRINTF("Failed to insert ocurrence");
			return (-1);
		case QR_TRYAGAIN:
			break;

		case QR_RESULT:
		default:
			EPRINTF("Unexpected query result (%d)", qr);
			break;
		}
	}
	return (0);
}

struct zs_ctx *
zs_new_ctx(void)
{
	struct zs_ctx *zc;

	zc = calloc(1, sizeof(*zc));
	if (zc == NULL)
		return (NULL);

	unlink("/tmp/zscope.sqlite3");
	if (sqlite3_open_v2("/tmp/zscope.sqlite3", &zc->zc_conn,
	    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
		EPRINTF("Unable to open database");
		return (NULL);
	}
	if (sqlite3_exec(zc->zc_conn, ZSC_FILETABLE, NULL, NULL, NULL) != SQLITE_OK)
		EPRINTF("Creating filetable: %s", sqlite3_errmsg(zc->zc_conn));
	if (sqlite3_exec(zc->zc_conn, ZSC_SYMBOLTABLE, NULL, NULL, NULL) != SQLITE_OK)
		EPRINTF("Creating symboltable: %s", sqlite3_errmsg(zc->zc_conn));
	if (sqlite3_exec(zc->zc_conn, ZSC_USAGETABLE, NULL, NULL, NULL) != SQLITE_OK)
		EPRINTF("Creating usagetable: %s", sqlite3_errmsg(zc->zc_conn));

	return (zc);
}

void
zs_free_ctx(struct zs_ctx *zc)
{
	sqlite3_close(zc->zc_conn);
	free(zc);
}
