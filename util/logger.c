#include "logger.h"

#include <stdio.h>
///TODO add synchronization

static log_level_t g_log_level = lvInfo;

void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message)
{
	if (level >= g_log_level)
		printf("%s(%d) %s: %s", file, line, func, message);
}

void log_set_level(log_level_t level)
{
	g_log_level = level;
}
