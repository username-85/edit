#include "display.h"
#include "slog.h"
#include "rc.h"
#include "util.h"
#include "utf.h"

#include <ctype.h>
#include <ncurses.h>
#include <string.h>

#define TAB_LEN 8

int display(struct buffer *buf)
{
	if (!buf)
		return ERROR;

	erase();

	int x = 0;
	int y = 0;
	int cursor_x = 0;
	int cursor_y = 0;
	char str[UTF_BUF_SIZE] = {0};
	char const *p = buf->disp_b;

	while (p <= buf->buf_e && y < LINES) {
		int x_inc = 0;

		attroff(A_REVERSE);
		if (buf->sel) {
			char *sel_b = buf->sel;
			char *sel_e = buf->cursor;

			if (buf->sel > buf->cursor) {
				sel_b = buf->cursor;
				sel_e = buf->sel;
			}

			if (sel_b <= p && p <= sel_e)
				attron(A_REVERSE);
		}

		if (buf->cursor == p) {
			cursor_x = x;
			cursor_y = y;
		}

		if (in_gap(buf, p)) {
			p = buf->gap_e;

			if (buf->cursor == p) {
				cursor_x = x;
				cursor_y = y;
			}

			if (p == buf->buf_e)
				continue;
		}

		int symb_size = get_symb_len(*p);
		if (!symb_size) {
			return ERROR;
		} else if (symb_size == 1) {
			if (isprint(*p)) {
				addch(*p);
				x_inc = 1;
			} else if (*p == '\n') {
				addch(*p);
				x_inc = 1;
			} else if (*p == '\t') {
				for (int i = 0; i < TAB_LEN; i++) {
					addch(' ');
				}
				x_inc = TAB_LEN;
			}
		} else {
			for (int i = 0; i < UTF_BUF_SIZE; i++)
				str[i] = '\0';

			for (int i = 0; i < symb_size; i++)
				str[i] = *(p + i);

			if (strlen(str)) {
				addstr(str);
				x_inc = 1;	
			}
		}

		x += x_inc;

		if (x == COLS || *p == '\n') {
			y++;
			x = 0;
		}

		p += symb_size;
	}
	buf->disp_e = (char *)p;

	move(cursor_y, cursor_x);
        refresh();

	return SUCCESS;
}

