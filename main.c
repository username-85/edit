#include "edit.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
	char *fname = NULL;
	if (argc > 1)
		fname = argv[1];

	struct buffer *buf = edit_prepare(fname);
	if (NULL == buf)
		exit(EXIT_FAILURE);

	edit_run(buf);
	edit_end(buf);

	exit(EXIT_SUCCESS);
}

