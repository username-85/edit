#include "buffer.h"
#include "operation.h"
#include "slog.h"
#include "rc.h"
#include "util.h"
#include "utf.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


static char * 
find_ch(struct buffer const *buf, char const *start_pos, char ch, 
	int direction);

static int 
count_symbols(struct buffer const *buf, char const *beg, char const *end);

static char * prev_symb(struct buffer const *buf, char const *pos); 
static char * next_symb(struct buffer const *buf, char const *pos);
static char * ptr_to_line_next(struct buffer const *buf, char const *pos);
static char * ptr_to_line_prev(struct buffer const *buf, char const *pos);
static int pos_in_line(struct buffer const *buf, char const *p);
static int this_line_len(struct buffer const *buf, char const *p); 

int mv_cursor(struct buffer *buf, int const direction) {
	if (!buf) {
		return ERROR;
	}

	char const *old_cur = buf->cursor;
	char const *new_cur = buf->cursor;

	if (DIR_NEXT == direction) {
		new_cur = next_symb(buf, old_cur);
	}
	else if (DIR_PREV == direction) {
		new_cur = prev_symb(buf, old_cur);
	}
	else if (DIR_LINEPREV == direction) {
		new_cur = ptr_to_line_prev(buf, old_cur);
	}
	else if (DIR_LINENEXT == direction) {
		new_cur = ptr_to_line_next(buf, old_cur);
	}
	else {
		return ERROR;
	}

	if (new_cur != old_cur
	    && new_cur >= buf->buf_b
	    && new_cur <= buf->buf_e) {

		buf->cursor = (char *)new_cur;

		if (buf->cursor < buf->disp_b) {
			buf->disp_b = ptr_to_line_prev(buf, buf->disp_b);
		}
		
		if (buf->cursor > buf->disp_e && buf->cursor != buf->gap_e) {
			buf->disp_b = ptr_to_line_next(buf, buf->disp_b);
		}
	}

	return SUCCESS;
}

char * find_ch(struct buffer const *buf, char const *start_pos, 
	       char const ch, int const direction) {
	if (DIR_NEXT != direction && DIR_PREV != direction) {
		return NULL;
	}

	char const *ret = NULL;
        char const *p = start_pos;
        while (buf->buf_b <= p && p <= buf->buf_e) {
		if (in_gap(buf, p)) {
			if (DIR_NEXT == direction) {
				p = buf->gap_e;
			}
			else {
				p = buf->gap_b;
				p--;
			}

			if (!in_buf(buf, p)) {
				break;
			}
			continue;
		}

        	if (*p == ch) {
        		ret = p;
        		break;
        	}
        	(DIR_NEXT == direction) ? p++ : p--;
        }

        return (char *)ret;
}

int add_ch(struct buffer *buf, char ch) {
	if (buf->gap_e == buf->gap_b) {
		if (increase_buffer(buf) == ERROR) {
			return ERROR;
		}
	}

	move_gap(buf);
	*buf->gap_b = ch;
	buf->gap_b++;

	return SUCCESS;
}

void del_symb(struct buffer *buf) {
	move_gap(buf);

	int bytes = get_symb_len(*buf->cursor);

	if (buf->gap_e + bytes <= buf->buf_e) {
		buf->gap_e += bytes;
		buf->cursor += bytes;
	}
}

void del_ch(struct buffer *buf) {
	move_gap(buf);

	if (buf->gap_e + 1 <= buf->buf_e) {
		buf->gap_e++;
		buf->cursor++;
	}
}

void del_prev_symb(struct buffer *buf) {
	move_gap(buf);

	char *prev_pos = prev_symb(buf, buf->cursor);
	if (prev_pos == buf->cursor) { 
		return;
	}

	int num_chars = get_symb_len(*prev_pos);
	buf->gap_b -= num_chars;
}

int count_symbols(struct buffer const *buf, char const *beg, char const *end) {
	int ret = 0;

	if (beg > end) {
		return 0;
	}
		
	char const *p, *p2;
	p = p2 = beg;
	do {
		p = p2;
		p2 = next_symb(buf, p);
		ret++;

	} while (p != p2 && p2 <= end);

	return ret;
}

char * prev_symb(struct buffer const *buf, char const *pos) {

	char const *new_pos = (char *)pos - 1;

	if (new_pos >= buf->gap_b && new_pos < buf->gap_e) {
		new_pos = buf->gap_b;
		new_pos--; 
	}

	if (new_pos < buf->buf_b) {
		return (char *)pos;
	}

	while(!ISASCII(*new_pos) && ISFILL(*new_pos)) {
		new_pos--;
		if (new_pos >= buf->gap_b && new_pos < buf->gap_e) {
			new_pos = buf->gap_b;
		}

		if (new_pos < buf->buf_b) {
			return (char *)pos;
		}
	}

	return (char *)new_pos;
}

char * next_symb(struct buffer const *buf, char const *pos) {

	int bytes_step = get_symb_len(*pos); 
	char const *new_pos = (char *)pos + bytes_step;

	if (new_pos >= buf->gap_b && new_pos < buf->gap_e) {
		return buf->gap_e;
	}

	if (new_pos > buf->buf_e) {
		return (char *)pos;
	}

	while(!ISASCII(*new_pos) && ISFILL(*new_pos)) {
		new_pos++;

		if (new_pos >= buf->gap_b && new_pos < buf->gap_e) {
			new_pos = buf->gap_e;
		}

		if (new_pos > buf->buf_e) {
			return (char *)pos;
		}

	}

	return (char *)new_pos;
}

char * ptr_to_line_b(struct buffer const *buf, char const *pos) {
	char const *p = pos;

	if ('\n' == *p) {
		p = prev_symb(buf, pos);
		if (p == pos) {
			return (char *)pos;
		}
	}

	char const *prev_line_e = find_ch(buf, p, '\n', DIR_PREV);
	if (prev_line_e) {
		return next_symb(buf, prev_line_e);
	}

	return buf->buf_b;
}

char * ptr_to_line_e(struct buffer const *buf, char const *p) {
	if ('\n' == *p) {
		return (char *)p;
	}
	char const *current_line_e = find_ch(buf, p, '\n', DIR_NEXT);
	if (current_line_e) {
		return (char *)current_line_e;
	}

	return buf->buf_e;
}

char * ptr_to_line_prev(struct buffer const *buf, char const *pos) {
	int line_pos = pos_in_line(buf, pos);
	char const *line_b = ptr_to_line_b(buf, pos);
	char const *line_prev_e = prev_symb(buf, line_b);
	char const *line_prev_b = ptr_to_line_b(buf, line_prev_e);
	int const line_prev_len = this_line_len(buf, line_prev_b);

	int line_offset = (line_prev_len > line_pos) 
	                   ? line_pos : line_prev_len;
	if (line_offset == line_prev_len
	    && line_offset == 1) {
		line_offset = 0;
	}

	char const *p, *p2;
	p = p2 = line_prev_b;
	for (int i = 0; i < line_offset; i++) {
		p = p2;
		p2 = next_symb(buf, p);
		if (p == p2) {
			break;
		}
	}
	return (char *)p2;
}

char * ptr_to_line_next(struct buffer const *buf, char const *pos) {
	char const *line_e = ptr_to_line_e(buf, pos);
	if (line_e == buf->buf_e) {
		return (char *)pos;
	}

	char const *line_next_b = next_symb(buf, line_e);
	int const line_next_len = this_line_len(buf, line_next_b);

	int line_pos = pos_in_line(buf, pos);
	int line_offset = (line_next_len > line_pos) 
	                   ? line_pos : line_next_len;
	if (line_offset == line_next_len && line_offset == 1) {
		line_offset = 0;
	}

	char const *p, *p2;
	p = p2 = line_next_b;
	for (int i = 0; i < line_offset; i++) {
		p = p2;
		p2 = next_symb(buf, p);
		if (p == p2) {
			break;
		}
	}
	return (char *)p2;
}

int pos_in_line(struct buffer const *buf, char const *p) {
	char const *line_b = ptr_to_line_b(buf, p);
	return count_symbols(buf, line_b, p) - 1;
}

int this_line_len(struct buffer const *buf, char const *p) {
	char const *line_b = ptr_to_line_b(buf, p);
	char const *line_e = ptr_to_line_e(buf, p);
	if (!line_b || !line_e) {
		return -1;
	}

	return count_symbols(buf, line_b, line_e);
}

int copy_sel(struct buffer *buf) {
	free_copy_buf(buf);
	if (!buf->sel) {
		return 0;
	}

	char *sel_b = buf->sel;
	char *sel_e = buf->cursor;
	if (buf->sel > buf->cursor) {
		sel_b = buf->cursor;
		sel_e = buf->sel;
	}

	int gap_b_sel = (sel_b <= buf->gap_b && buf->gap_b <= sel_e);
	int gap_e_sel = (sel_b <= buf->gap_e && buf->gap_e <= sel_e);
	int skip_gap_size = 0;

	if (gap_b_sel && gap_e_sel) {
		skip_gap_size = buf->gap_e - buf->gap_b;
	}
	else if (gap_b_sel) {
		skip_gap_size = sel_e - buf->gap_b;
	}
	else if (gap_e_sel) {
		skip_gap_size = buf->gap_e - sel_b;
	}

	int buf_size = sel_e - sel_b - skip_gap_size;
	if (buf_size <= 0) {
		return 0;
	}

	char *buf_p = calloc(buf_size, sizeof(char));
	if (!buf_p) {
		return 0;
	}
	buf->copy_buf = buf_p;
	buf->copy_buf_size = buf_size; 

	int i = 0;
	char *p = sel_b;
	for (i = 0; i < buf_size && p <= sel_e; i++, p++) {

		if (in_gap(buf, p) && buf->gap_e < sel_e) {
			p = buf->gap_e;
		}
		buf->copy_buf[i] = *p;
	}

	return i;
}

int paste(struct buffer *buf) {
	bool ok = true;
	for (size_t i = 0; i < buf->copy_buf_size && ok; i++) {
		ok = (add_ch(buf, buf->copy_buf[i]) == SUCCESS) ;
	}
	return (ok ? SUCCESS : ERROR);
}

void del_sel(struct buffer *buf) {
	if (!buf->sel) {
		return;
	}

	char *sel_b = buf->sel;
	char *sel_e = buf->cursor;
	if (buf->sel > buf->cursor) {
		sel_b = buf->cursor;
		sel_e = buf->sel;
	}

	buf->cursor = sel_b;
	move_gap(buf);

	int bytes = sel_e - sel_b;
	while (bytes-- > 0) {
		del_ch(buf);
	}
}


void mv_by_lines(struct buffer *buf, int lines_num, int direction) {
	char *cur_prev = buf->cursor;
	char *cur_now = buf->cursor;
	for (int i = 0; i < lines_num ; i++) {
		if (direction == DIR_LINENEXT) {
			cur_now = ptr_to_line_next(buf, cur_prev);
		}
		else {
			cur_now = ptr_to_line_prev(buf, cur_prev);
		}

		if (cur_now == cur_prev) {
			break;
		}
		cur_prev = cur_now;
	}
	if (cur_now != buf->cursor) {
		buf->cursor = cur_now;
		buf->disp_b = ptr_to_line_b(buf, cur_now);
	}
}
