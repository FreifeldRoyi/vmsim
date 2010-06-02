#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

#include <sys/syscall.h>
#include <unistd.h>

pid_t cur_tid()
{
	return syscall(SYS_gettid);
}

///TODO add synchronization

static log_level_t g_log_level = lvError;

void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message,...)
{
	va_list arglist;
	va_start(arglist, message);
	if (level >= g_log_level)
	{
		printf("[%d]%s(%d) %s:", cur_tid(), file, line, func);
		vprintf(message, arglist);
	}
	va_end(arglist);
}

void log_set_level(log_level_t level)
{
	g_log_level = level;
}
