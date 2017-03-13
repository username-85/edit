#ifndef SLOG_H
#define SLOG_H

#define LOGFILE "edit.log"
#define STR_BUF_LENGTH 20
#define WITH_TIME 1
#define WITHOUT_TIME 0

#include <stdio.h>

// integer to string
int int_to_str(int const value, char *ptr); 

// add string that consist of startng tag and message. 
// with or without time depending on time_opt parameter
int slog(char const *tag, char const *message, 
          char const *filename, int time_opt);

// some wrappers for slog, that would write to LOGFILE
int log_ss(char const *tag, char const *str); 
int log_si(char const *tag, int value); 
int log_sc(char const *tag, char c);

#endif /* SLOG_H */
