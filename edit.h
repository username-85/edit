#ifndef EDIT_H
#define EDIT_H
#include <stdio.h>

// prepare terminal, load file in buffer for edit or create empty new buffer
struct buffer * edit_prepare(char const *fname);

// main editor loop
int edit_run(struct buffer *buf);

// frees memory, finishing ncurses mode
int edit_end(struct buffer *buf);

#endif /* EDIT_H */
