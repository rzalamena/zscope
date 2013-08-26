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
