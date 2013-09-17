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

#include <sys/types.h>

#include <dirent.h>
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <zscope.h>

const char *zs_file;
struct zs_ctx *zc_parsectx;

extern FILE *yyin;
int yyparse(void);

static int zscope_parse_file(const char *);
static int zscope_parse_dir(const char *);

static int
zscope_parse_file(const char *file)
{
	zs_file = file;

	DPRINTF("Parsing file '%s'", file);

	yyin = fopen(file, "r");
	if (yyin == NULL) {
		EPRINTF("fopen: %m");
		return (-1);
	}

	while (yyparse() == 0)
		/* NOTHING */;

	fclose(yyin);

	return (0);
}

static int
zscope_parse_dir(const char *directory)
{
	DIR	*dp;
	struct	 dirent *d, *dr;
	size_t	 dlen;
	char	 dpath[1024];

	DPRINTF("Parsing directory '%s'...", directory);

	dp = opendir(directory);
	if (dp == NULL) {
		EPRINTF("opendir: %m");
		return (-1);
	}

	dlen = offsetof(struct dirent, d_name) +
	    pathconf(directory, _PC_NAME_MAX) + 1;

	d = calloc(1, dlen);
	if (d == NULL) {
		EPRINTF("calloc: %m");
		closedir(dp);
		return (-1);
	}

	while ((readdir_r(dp, d, &dr) == 0) && (dr != NULL)) {
		if (d->d_name[0] == '.')
			continue;

		if (d->d_type == DT_DIR) {
			snprintf(dpath, sizeof(dpath), "%s/%s",
			    directory, d->d_name);
			if (zscope_parse_dir(dpath) != 0)
				EPRINTF("Sub-directory '%s' parse failed",
				    d->d_name);

			continue;
		}

		if (d->d_type != DT_REG) {
			DPRINTF("'%s' is not a regular file (%d)",
			    d->d_name, d->d_type);

			continue;
		}

		dlen = strlen(d->d_name);
		if (d->d_name[dlen - 2] != '.' &&
		    (d->d_name[dlen - 1] != 'c' ||
		    d->d_name[dlen - 1] != 'h')) {
			DPRINTF("'%s' is not a source file", d->d_name);
			continue;
		}

		zscope_parse_file(d->d_name);
	}
	closedir(dp);
	free(d);

	return (0);
}

int
main(int argc, char *argv[])
{
	zc_parsectx = zs_new_ctx();
	if (zc_parsectx == NULL)
		err(1, "main");

	zscope_parse_dir("/tmp");

	zs_free_ctx(zc_parsectx);

	exit(EXIT_SUCCESS);
}
