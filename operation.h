#ifndef OPERATION_H
#define OPERATION_H

#define DIR_PREV 0
#define DIR_NEXT 1
#define DIR_LINEPREV 3
#define DIR_LINENEXT 4

// cursor movement in buffer
int mv_cursor(struct buffer *buf, int direction);

// adding byte to buffer
int add_ch(struct buffer *buf, char ch);

// deleting symbol at cursor position (it maybe few bytes) from buffer
void del_symb(struct buffer *buf);

// deleting symbol before cursor position (it maybe few bytes) from buffer
void del_prev_symb(struct buffer *buf);

// copy bytes that are between cursor and selection start to copy buffer
int copy_sel(struct buffer *buf);

// paste bytes from copy buffer to cursor point
int paste(struct buffer *buf);

// delete bytes that are between cursor and selection start 
void del_sel(struct buffer *buf);

// move cursor point by lines number in direction
void mv_by_lines(struct buffer *buf, int lines_num, int direction);

// returns pointer to begin of line
char * ptr_to_line_b(struct buffer const *buf, char const *start_pos);

// returns pointer to end of line 
char * ptr_to_line_e(struct buffer const *buf, char const *start_pos);

#endif /* OPERATION_H */
