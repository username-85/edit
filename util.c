#include "util.h"
#include "utf.h"
#include <stdio.h>

size_t get_fsize(char const *fname) {
	FILE * f = fopen(fname, "r");
	fseek(f, 0, SEEK_END);
	size_t size = (size_t)ftell(f);
	fclose(f);
	return size;
}

size_t get_symb_len(char first_ch) {
	size_t ret = 0;

	if (ISASCII(first_ch)) {
		ret = 1;
	}
	else {
		ret = UTF8LEN(first_ch);
	}
	return ret;
}

int file_exists(const char *fname) {
	FILE *file;
	if (( file = fopen(fname, "r") )) {
		fclose(file);
		return 1;
	}
	return 0;
}
