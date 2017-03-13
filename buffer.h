#ifndef BUFFER_H 
#define BUFFER_H 

#define FNAMELEN_MAX 70
#define INIT_BUF_SIZE 1024
#define INC_BUF_SIZE 1024
#include <stdio.h>

struct buffer {
	char *disp_b;        // first displayed byte
	char *disp_e;        // byte right after last displayed byte
        size_t size;         // size of buffer
        char *cursor;        // cursor position in buffer
        char *buf_b;         // first byte in buffer
        char *buf_e;         // byte rigth after the last one buffer byte
        char *gap_b;         // start of gap
        char *gap_e;         // byte right after the last one byte in gap
        char *sel;           // selection start
        char *copy_buf;      // start of copy buffer
        size_t copy_buf_size;
        char filename[FNAMELEN_MAX];
}; 

struct buffer* create_buffer(void);

// return memory that was used for buffer and copy buffer
void delete_buffer(struct buffer *buf); 

// increase buffer by INC_BUF_SIZE 
int increase_buffer(struct buffer *buf);

// load file to buffer and increase the buffer if nessessary
int load_file(struct buffer *buf, char const *fname);

int in_buf(struct buffer const *buf, char const *pos);
int in_gap(struct buffer const *buf, char const *pos); 

// move gap inside buffer
void move_gap(struct buffer *buf); 

// saving buffer to file
int save(struct buffer const *buf);

// return memory that was used for copy buffer
void free_copy_buf(struct buffer *buf);

// toggle selection mode on/off
void toggle_selection(struct buffer *buf);

// some functions that was used for debug
void log_buf(struct buffer const *buf);
void log_buf_ch(struct buffer *buf, int num_chars);

#endif /* BUFFER_H */
