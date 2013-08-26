#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <zscope.h>

const char *zs_file;
struct zs_ctx *zc_parsectx;

int
main(int argc, char *argv[])
{
	zs_file = "/tmp/main.c";
	zc_parsectx = zs_new_ctx();
	if (zc_parsectx == NULL)
		err(1, "main");

	zs_free_ctx(zc_parsectx);

	exit(EXIT_SUCCESS);
}
