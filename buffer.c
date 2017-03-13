#include "buffer.h"
#include "util.h"
#include "slog.h"
#include "rc.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

static int inc_buffer_by_size(struct buffer *buf, size_t const inc_size);

struct buffer* create_buffer(void) {

	size_t const buf_size = INIT_BUF_SIZE;
	struct buffer *buf= calloc(1, sizeof(struct buffer));
	if (!buf) {
		return NULL;
	}

	buf->buf_b = calloc(buf_size, sizeof(char));
	if (!buf->buf_b) {
		free(buf);
		return NULL;
	}

	buf->disp_b = buf->buf_b;
	buf->disp_e = NULL;
	buf->buf_e = buf->buf_b + buf_size;
	buf->gap_b = buf->buf_b;
	buf->gap_e = buf->gap_b + buf_size;
	buf->cursor = buf->gap_e;
	buf->size = buf_size;
	buf->sel = NULL;
	buf->copy_buf = NULL;
	buf->copy_buf_size = 0;
	strncpy(buf->filename, "\0", FNAMELEN_MAX);

	return buf;
}

void delete_buffer(struct buffer *buf) {
	if (!buf) { 
		return; 
	}

	if (buf->buf_b) {
		free(buf->buf_b);
	}

	free_copy_buf(buf);

	free(buf);
}

void log_buf(struct buffer const *buf) {
	log_ss("log_buf b", "-------------------------------------");
	log_si("buf", (int)(uintptr_t)buf);
	log_si("buf->size", buf->size);
	log_si("gap size",  buf->gap_e - buf->gap_b);
	log_si("buf->cursor", (int)(uintptr_t)buf->cursor);
	log_si("buf->buf_b", (int)(uintptr_t)buf->buf_b);
	log_si("buf->buf_e", (int)(uintptr_t)buf->buf_e);
	log_si("buf->gap_b", (int)(uintptr_t)buf->gap_b);
	log_si("buf->gap_e", (int)(uintptr_t)buf->gap_e);
	log_ss("buf->filename", buf->filename);
	log_si("buf->disp_b", (int)(uintptr_t)buf->disp_b);
	log_si("buf->disp_e", (int)(uintptr_t)buf->disp_e);
	log_si("buf->sel", (int)(uintptr_t)buf->sel);
	log_si("buf->copy_buf", (int)(uintptr_t)buf->copy_buf);
	log_si("buf->copy_buf_size", buf->copy_buf_size);
	log_ss("log_buf e", "-------------------------------------");
}

int increase_buffer(struct buffer *buf) {
	return inc_buffer_by_size(buf, INC_BUF_SIZE); 
}

int inc_buffer_by_size(struct buffer *buf, size_t const inc_size) {

	int before_gap_size = buf->gap_b - buf->buf_b;
	int after_gap_size = buf->buf_e - buf->gap_e;
	int gap_size = buf->gap_e - buf->gap_b;
	int disp_b_pos = buf->disp_b - buf->buf_b;

	int sel_index = -1;
	if (buf->sel) {
		sel_index = buf->sel <= buf->gap_b
	                    ? buf->sel - buf->buf_b
	                    : buf->sel - gap_size - buf->buf_b;
	}

	int cursor_index = buf->cursor <= buf->gap_b
	                   ? buf->cursor - buf->buf_b
	                   : buf->cursor - gap_size - buf->buf_b;

	char *buf_inc = realloc(buf->buf_b, 
	                        sizeof(char) * (buf->size + inc_size));
	if (!buf_inc) {
		return ERROR;
	}

	buf->buf_b = buf_inc;
	buf->size += inc_size;
	buf->buf_e = buf->buf_b + buf->size;
	buf->gap_b = buf->buf_b + before_gap_size;
	buf->gap_e = buf->gap_b + gap_size;
	buf->disp_b = buf->buf_b + disp_b_pos;

	buf->cursor = (buf->buf_b + cursor_index) <= buf->gap_b
	              ? buf->buf_b + cursor_index
	              : buf->gap_e + (cursor_index - before_gap_size);

	if (sel_index >= 0) {
		buf->sel= (buf->buf_b + sel_index) <= buf->gap_b
		          ? buf->buf_b + sel_index
	                  : buf->gap_e + (sel_index - before_gap_size);
	}

	memmove(buf->gap_e + inc_size, buf->gap_e, 
	        sizeof(char) * after_gap_size);

	buf->gap_e += inc_size;

	return SUCCESS;
}

int load_file(struct buffer *buf, char const *fname) {
	if (!buf || !fname || !strlen(fname)) {
		return ERROR;
	}

	FILE *fp;
	fp = fopen(fname, "r");
	if (!fp) {
		return ERROR;
	}

	size_t fsize;
	fsize = get_fsize(fname);
	if (inc_buffer_by_size(buf, fsize) != SUCCESS) {
		fclose(fp);
		return ERROR;
	}
	
	int bytes_read = fread(buf->buf_b, sizeof(char), fsize , fp);
	if (bytes_read <= 0) {
		fclose(fp);
		return ERROR;
	}
	buf->gap_b += bytes_read;

	strncpy(buf->filename, fname, FNAMELEN_MAX);
	fclose(fp);
	return SUCCESS;
}

int in_buf(struct buffer const *buf, char const *pos) {
	return (buf->buf_b <= pos && pos < buf->buf_e);
}

int in_gap(struct buffer const *buf, char const *pos) {
	return (buf->gap_b <= pos && pos < buf->gap_e);
}


void move_gap(struct buffer *buf) {

	if (buf->cursor == buf->gap_e) {
		return;
	}

	if (buf->cursor == buf->gap_b) {
		buf->cursor = buf->gap_e;
		return;
	}

	if (buf->cursor < buf->gap_b) {
		int chunk_size = buf->gap_b - buf->cursor;

		buf->gap_b -= chunk_size;
		buf->gap_e -= chunk_size;
		buf->cursor = buf->gap_e;

		memmove(buf->gap_e, buf->gap_b, sizeof(char) * chunk_size); 
	}
	else {
		int chunk_size = buf->cursor - buf->gap_e;

		memmove(buf->gap_b, buf->gap_e, sizeof(char) * chunk_size);

		buf->gap_b += chunk_size;
		buf->gap_e = buf->cursor;
	}

}

void log_buf_ch(struct buffer *buf, int num_chars) {
	for (int i = 0; i < num_chars; i++) {
		log_sc("log_buf_ch", buf->buf_b[i]);
	}
}

int save(struct buffer const *buf) {
        FILE *fp;

        fp = fopen(buf->filename, "w");
        if (!fp) {
                log_ss("error", "save fopen fail");
                return ERROR;
        }

        size_t length;
        length = buf->gap_b - buf->buf_b;
	if (length) {
        	if (fwrite(buf->buf_b, sizeof (char), length, fp) != length) {
                	log_ss("error", "save wrong length before gap");
                	return ERROR;
        	}
	}

        length = buf->buf_e - buf->gap_e;
	if (length) {
        	if (fwrite(buf->gap_e, sizeof (char), length, fp) != length) {
                	log_ss("error", "save wrong length after gap");
                	return ERROR;
        	}
	}

        if (fclose(fp) != 0) {
                log_ss("error", "save flcose");
                return ERROR;
        }

        return SUCCESS;
}

void toggle_selection(struct buffer *buf) {
	if (!buf->sel) {
		buf->sel = buf->cursor;
	}
	else {
		buf->sel = NULL;
	}
}

void free_copy_buf(struct buffer *buf) {
	buf->copy_buf_size = 0;
	if (buf->copy_buf) {
		free(buf->copy_buf);
	}
}
