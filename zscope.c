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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <zscope.h>

const char *zs_file;
struct zs_ctx *zc_parsectx;

extern FILE *yyin;
int yyparse(void);

int
main(int argc, char *argv[])
{
	zs_file = "/tmp/main.c";
	zc_parsectx = zs_new_ctx();
	if (zc_parsectx == NULL)
		err(1, "main");

	yyin = fopen(zs_file, "r");
	if (yyin == NULL)
		err(1, "fopen");

	while (yyparse() != 0);

	fclose(yyin);

	zs_free_ctx(zc_parsectx);

	exit(EXIT_SUCCESS);
}
