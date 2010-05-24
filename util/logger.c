#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
///TODO add synchronization

static log_level_t g_log_level = lvInfo;

void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message,...)
{
	va_list arglist;
	va_start(arglist, message);
	if (level >= g_log_level)
	{
		printf("%s(%d) %s:", file, line, func);
		vprintf(message, arglist);
	}
	va_end(arglist);
}

void log_set_level(log_level_t level)
{
	g_log_level = level;
}
