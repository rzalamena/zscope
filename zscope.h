#ifndef __ZSCOPE_H__
#define __ZSCOPE_H__

#include <sys/queue.h>

#define EPRINTF(fmt, args...) \
	do { \
		fprintf(stderr, "(%s:%d) " fmt "\n", \
		    __FILE__, __LINE__, ## args); \
	} while (0)

#define ZSCOPE_DEBUG (0)
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

extern size_t zs_line;
extern const char *zs_file;
extern int column;
extern struct zs_ctx *zc_parsectx;

#endif /* __ZSCOPE_H__ */