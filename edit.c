#include "buffer.h"
#include "display.h"
#include "edit.h"
#include "operation.h"
#include "slog.h"
#include "rc.h"
#include "util.h"
#include "utf.h"

#include <locale.h>
#include <ncurses.h>
#include <stdbool.h>
#include <string.h>

#define ALT_BACKSPACE 127 
#define KEY_ESC 27

static void get_input(char * prompt, char *input, size_t size);
static void msg(char const * msg);
static int save_to_file(struct buffer *buf);
static void cut_selection(struct buffer *buf);
static int paste_selection(struct buffer *buf);
static void copy_selection(struct buffer *buf);
static void delete(struct buffer *buf);
static void delete_prev(struct buffer *buf);

// could move cursor too far if lines are long 
static void pg_down(struct buffer *buf);
static void pg_up(struct buffer *buf);

static void home(struct buffer *buf);
static void end(struct buffer *buf);
static int add_symbol(struct buffer *buf, char ch);

static int term_init(void); 

struct buffer * edit_prepare(char const *fname)
{
	if (term_init() != SUCCESS) {
		log_ss("error", "term_init fail");
		return NULL;
	}

	struct buffer *buf = create_buffer();
	if (!buf) {
		log_ss("error", "create_buffer fail");
		msg("Error. Details in " LOGFILE);
		getch();
		return NULL;
	}

	if (fname) {
		if (file_exists(fname)) {
			if (load_file(buf, fname) == ERROR) {
				log_ss("error", "load_file fail");
				msg("Error. Details in " LOGFILE);
				getch();
				return NULL; 
			}
		} else {
			strncpy(buf->filename, fname, FNAMELEN_MAX);
		}
	}

	return buf;
}

int edit_run(struct buffer *buf)
{
	if (!buf)
		return ERROR;

	if (display(buf) != SUCCESS) {
		log_ss("error", "edit_run display fail");
		msg("Error. Details in " LOGFILE);
		getch();
		return ERROR;
	}

	move(0, 0);
	char const *help_str = "F1-Help  F2-Save   F3-Sel(on/off)  "
	                       "F4-Copy  F5-Cut  F6-Paste  F10-Quit  "
	                       "(any key - to continue)";
	msg(help_str);

	int ch;
	bool in_loop = true;
	bool err = false;
    	while(in_loop) {
    		bool redisplay = true;

		switch(ch = getch()) {
		case KEY_F(10):
			in_loop = false;
			break;
    		case KEY_RIGHT:
			mv_cursor(buf, DIR_NEXT);
			break;
		case KEY_LEFT:
			mv_cursor(buf, DIR_PREV);
			break;
		case KEY_DOWN:
			mv_cursor(buf, DIR_LINENEXT);
			break;
		case KEY_UP:
			mv_cursor(buf, DIR_LINEPREV);
			break;
		case KEY_BACKSPACE:
		case ALT_BACKSPACE:
			delete_prev(buf);
			break;
		case KEY_DC:
    			delete(buf);
    			break;
    		case KEY_ESC:
    			buf->sel =  NULL;
    			break;
    		case KEY_F(1):
			msg(help_str);
			redisplay = 0;
			break;
		case KEY_F(2):
			save_to_file(buf);
			redisplay = 0;
			break;
		case KEY_F(3):
			toggle_selection(buf);
			break;
		case KEY_F(4):
			copy_selection(buf);
			break;
		case KEY_F(5):
			cut_selection(buf);
			break;
		case KEY_F(6):
			if (paste_selection(buf) == ERROR) {
				log_ss("error", "paste_selection fail");
				in_loop = false;
				err = true;
			}
			break;
		case KEY_NPAGE:
			pg_down(buf);
			break;
		case KEY_PPAGE:
			pg_up(buf);
			break;
		case KEY_HOME:
			home(buf);
			break;
		case KEY_END:
			end(buf);
			break;
		default:
			if (add_symbol(buf, ch) == ERROR) {
				log_ss("error", "add_symbol fail");
				in_loop = false;
				err = true;
				continue;
			}
		}

		if (redisplay)
			display(buf);
    	}
	
	if (err) {
		msg("Error. Details in " LOGFILE);
		getch();
		return ERROR;
	}

	return SUCCESS;
}

int edit_end(struct buffer *buf)
{
        endwin();
	if (buf) { 
		delete_buffer(buf);
		return SUCCESS;
	} 

	return ERROR;
}

static int term_init(void)
{
	setlocale(LC_ALL, "");
        initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	set_escdelay(20);

	return SUCCESS;
}

static void get_input(char * prompt, char *input, size_t size)
{
	attron(A_REVERSE);
	echo();
	move(LINES - 1, 0);         
	for (int i = 0; i < COLS; i++) {
		addch(' ');
	}
	mvaddnstr(LINES - 1, 0, prompt, COLS);
	getnstr(input, size);
	noecho();
	attroff(A_REVERSE);
}

static int save_to_file(struct buffer *buf)
{
	while (!strlen(buf->filename))
		get_input("Enter filename: ", buf->filename, FNAMELEN_MAX);
	
	if (save(buf) == SUCCESS) {
		msg("File saved");
		return SUCCESS;
	} else {
		msg("Error! File wasn't saved");
		return ERROR;
	}
}

static void msg(char const * msg)
{
	attron(A_REVERSE);
	echo();
	move(LINES - 1, 0);         
	for (int i = 0; i < COLS; i++) {
		addch(' ');
	}
	move(LINES - 1, 0);         
	addnstr(msg, COLS);
	move(LINES - 1, COLS - 1);
	noecho();
	attroff(A_REVERSE);
}

static void cut_selection(struct buffer *buf)
{
	if (buf->sel) {
		copy_sel(buf);
		del_sel(buf);
		buf->sel = NULL;
	}
}

static int paste_selection(struct buffer *buf)
{
	bool ok = true;
	if (buf->copy_buf && buf->copy_buf_size) {
		ok = (paste(buf) == SUCCESS);
		buf->sel = NULL;
	}
	return (ok ? SUCCESS : ERROR);
}

static void copy_selection(struct buffer *buf)
{
	if (buf->sel) {
		copy_sel(buf);
		buf->sel = NULL;
	}
}

static void delete(struct buffer *buf)
{
	if (buf->sel) {
		del_sel(buf);
		buf->sel = NULL;
	} else {
		del_symb(buf);
	}
}

static void delete_prev(struct buffer *buf)
{
	if (buf->sel) {
		del_sel(buf);
		buf->sel = NULL;
	} else {
		del_prev_symb(buf);
	}
}

static void pg_down(struct buffer *buf)
{
	mv_by_lines(buf, LINESONPAGE, DIR_LINENEXT);
}

static void pg_up(struct buffer *buf)
{
	mv_by_lines(buf, LINESONPAGE, DIR_LINEPREV);
}

static void home(struct buffer *buf)
{
	buf->cursor = ptr_to_line_b(buf, buf->cursor);
}

static void end(struct buffer *buf)
{
	buf->cursor = ptr_to_line_e(buf, buf->cursor);
}

static int add_symbol(struct buffer *buf, char ch)
{
	bool ok = (add_ch(buf, ch) == SUCCESS);
	if (ok) {
		int symb_len = get_symb_len(ch); 
		for (int i = 1; i < symb_len && ok; i++) {
			ch = getch();
			ok = (add_ch(buf, ch) == SUCCESS);
		}
	}

	return (ok ? SUCCESS : ERROR);
}
