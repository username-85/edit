#include "slog.h"
#include <time.h>

int slog(char const *tag, char const *message, 
          char const *filename, int time_opt)
{
	FILE *fp = fopen(filename, "a");
	if (!fp) 
		return -1;

	int with_time = (time_opt == WITH_TIME);
	if (with_time) {
		time_t now;
		time(&now);
		fprintf(fp, "%s [%s]: %s\n", ctime(&now), tag,
		        message);
	} else {
		fprintf(fp, "[%s]: %s\n", tag, message);
	}
	fclose(fp);

	return 0;
}

int int_to_str(int value, char *ptr)
{
	int count = 0;
	if (!ptr)
		return 0;

	if (0 == value) {
		ptr[0] = '0';
		ptr[1] = '\0';
		return 1;
	}

	if (value < 0) {
		value *= (-1);
		*ptr++ = '-';
		count++;
	}

	for (int tmp = value; tmp > 0; tmp /= 10, ptr++) {
	}
	*ptr = '\0';

	for (int tmp = value; tmp > 0; tmp /= 10) {
		*--ptr = tmp % 10 + '0';
		count++;
	}

	return count;
}

int log_si(char const *tag, int const value)
{
	char str[STR_BUF_LENGTH] = {0};
	int_to_str(value, str); 

	return slog(tag, str, LOGFILE, WITHOUT_TIME);
}

int log_ss(char const *tag, char const *str)
{
	return slog(tag, str, LOGFILE, WITHOUT_TIME);
}

int log_sc(char const *tag, char const c)
{
	char str[2] = {c, '\0'};
	return slog(tag, str, LOGFILE, WITHOUT_TIME);
}
