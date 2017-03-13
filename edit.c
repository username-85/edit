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

int term_init(void) {
	setlocale(LC_ALL, "");
        initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	set_escdelay(20);

	return SUCCESS;
}

struct buffer * edit_prepare(char const *fname) {
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
		}
		else {
			strncpy(buf->filename, fname, FNAMELEN_MAX);
		}
	}

	return buf;
}

int edit_end(struct buffer *buf) {
        endwin();
	if (buf) { 
		delete_buffer(buf);
		return SUCCESS;
	}
	else {
		return ERROR;
	}
}

int edit_run(struct buffer *buf) {

	if (!buf) {
		return ERROR;
	}

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
	bool ok = true;
    	while(ok) {
    		bool redisplay = true;
		ch = getch();

    		if (KEY_F(10) == ch) {
    			break;
    		}
		else if (KEY_RIGHT == ch) {
			mv_cursor(buf, DIR_NEXT);
		}
		else if (KEY_LEFT == ch) {
			mv_cursor(buf, DIR_PREV);
		}
    		else if (KEY_DOWN == ch) {	
			mv_cursor(buf, DIR_LINENEXT);
		}
		else if (KEY_UP == ch) {
			mv_cursor(buf, DIR_LINEPREV);
		}
		else if (KEY_BACKSPACE == ch || ALT_BACKSPACE == ch) {
			delete_prev(buf);
		} 
    		else if(KEY_DC == ch) {
    			delete(buf);
    		}
    		else if(KEY_ESC == ch) {
    			buf->sel =  NULL;
    		}
		else if (KEY_F(1)== ch) {
			msg(help_str);
			redisplay = 0;
		} 
		else if(KEY_F(2) == ch) {
			save_to_file(buf);
			redisplay = 0;
		}
		else if (KEY_F(3)== ch) {
			toggle_selection(buf);
		}
		else if (KEY_F(4)== ch) {
			copy_selection(buf);
		}
		else if (KEY_F(5)== ch) {
			cut_selection(buf);
		}
		else if (KEY_F(6)== ch) {
			if (paste_selection(buf) == ERROR) {
				log_ss("error", "paste_selection fail");
				ok = false;
				continue;
			}
		}
		else if (KEY_NPAGE == ch) {
			pg_down(buf);
		}
		else if (KEY_PPAGE == ch) {
			pg_up(buf);
		}
		else if (KEY_HOME == ch) {
			home(buf);
		}
		else if (KEY_END == ch) {
			end(buf);
		}
		else {
			if (add_symbol(buf, ch) == ERROR) {
				log_ss("error", "add_symbol fail");
				ok = false;
				continue;
			}
		}

		if (redisplay) {
			display(buf);
		}
    	}
	
	if (!ok) {
		msg("Error. Details in " LOGFILE);
		getch();
		return ERROR;
	}
	return SUCCESS;
}

void get_input(char * prompt, char *input, size_t size) {
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

int save_to_file(struct buffer *buf) {
	while (!strlen(buf->filename)) {
		get_input("Enter filename: ", buf->filename, FNAMELEN_MAX);
	}
	
	if (save(buf) == SUCCESS) {
		msg("File saved");
		return SUCCESS;
	}
	else {
		msg("Error! File wasn't saved");
		return ERROR;
	}
}

void msg(char const * msg) {
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

void cut_selection(struct buffer *buf) {
	if (buf->sel) {
		copy_sel(buf);
		del_sel(buf);
		buf->sel = NULL;
	}
}

int paste_selection(struct buffer *buf) {
	bool ok = true;
	if (buf->copy_buf && buf->copy_buf_size) {
		ok = (paste(buf) == SUCCESS);
		buf->sel = NULL;
	}
	return (ok ? SUCCESS : ERROR);
}

void copy_selection(struct buffer *buf) {
	if (buf->sel) {
		copy_sel(buf);
		buf->sel = NULL;
	}
}

void delete(struct buffer *buf) {
	if (buf->sel) {
		del_sel(buf);
		buf->sel = NULL;
	}
	else {
		del_symb(buf);
	}
}

void delete_prev(struct buffer *buf) {
	if (buf->sel) {
		del_sel(buf);
		buf->sel = NULL;
	}
	else {
		del_prev_symb(buf);
	}
}

void pg_down(struct buffer *buf) {
	mv_by_lines(buf, LINESONPAGE, DIR_LINENEXT);
}


void pg_up(struct buffer *buf) {
	mv_by_lines(buf, LINESONPAGE, DIR_LINEPREV);
}


void home(struct buffer *buf) {
	buf->cursor = ptr_to_line_b(buf, buf->cursor);
}

void end(struct buffer *buf) {
	buf->cursor = ptr_to_line_e(buf, buf->cursor);
}

int add_symbol(struct buffer *buf, char ch) {
	bool ok = (add_ch(buf, ch) == SUCCESS) ;
	if (ok) {
		int symb_len = get_symb_len(ch); 
		for (int i = 1; i < symb_len && ok; i++) {
			ch = getch();
			ok = (add_ch(buf, ch) == SUCCESS);
		}
	}
	return (ok ? SUCCESS : ERROR);

}
