#ifndef UTIL_H 
#define UTIL_H 
#include <stdio.h>

size_t get_fsize(char const *fname);

// takes first byte of symbol and return number of bytes for that symbol
// it should be 1 for ascii and could more then 1 for utf symbol
size_t get_symb_len(char first_ch);

int file_exists(const char *fname);

#endif /* UTIL_H */
