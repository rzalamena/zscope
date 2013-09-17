/*
 * Copyright (c) 2013 Rafael F. Zalamena <rzalamena@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __ZSCOPE_H__
#define __ZSCOPE_H__

#include <sys/queue.h>

#include <string.h>

#define EPRINTF(fmt, args...) \
	do { \
		fprintf(stderr, "(%s:%d) " fmt "\n", \
		    __FILE__, __LINE__, ## args); \
	} while (0)

#define ZSCOPE_DEBUG (1)
#define DPRINTF(fmt, args...) \
	do { \
		if (ZSCOPE_DEBUG) \
			fprintf(stderr, "(%s:%d) " fmt "\n", \
			    __FILE__, __LINE__, ## args); \
	} while (0)

struct zitem {
	const char		*zi_filename;
	size_t			 zi_line;
	int			 zi_isdefinition;
	enum zitem_type {
		ZT_UNKNOWN	= 0,
		ZT_FUNCTION	= 1,
		ZT_VARIABLE	= 2,
		ZT_DEFINE	= 3,
		ZT_STRING	= 4,
	} zi_type;

	CIRCLEQ_ENTRY(zitem)	zi_entry;
};

struct zresult {
	size_t			zr_resultcount;

	CIRCLEQ_HEAD(, zitem)	zr_itemlist;
};

struct zs_ctx;

int zs_register_symbol(struct zs_ctx *, const char *, size_t, const char *, enum zitem_type, int);
struct zs_ctx *zs_new_ctx(void);
void zs_free_ctx(struct zs_ctx *);

/* Parser stuff */
typedef struct {
	union {
		char		*string;
	} v;
} YYSTYPE;

extern YYSTYPE yylval;

extern size_t zs_line;
extern const char *zs_file;
extern int column;
extern struct zs_ctx *zc_parsectx;

#endif /* __ZSCOPE_H__ */
