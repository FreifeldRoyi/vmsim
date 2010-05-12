#ifndef LOGGER_H_
#define LOGGER_H_

typedef enum {lvInfo, lvError} log_level_t;

#define LOG(_lvl, _msg) log_write(__FILE__, __LINE__, __func__, _lvl, _msg)
#define INFO(_msg) LOG(lvInfo, _msg)
#define ERROR(_msg) LOG(lvError, _msg)

void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message);

void log_set_level(log_level_t level);

#endif /* LOGGER_H_ */
