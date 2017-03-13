#ifndef DISPLAY_H 
#define DISPLAY_H 
#include "buffer.h"
#define LINESONPAGE LINES/2

// displays (some) of  the buffer content on screen
int display(struct buffer *buf);

#endif /* DISPLAY_H */
