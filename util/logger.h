#ifndef LOGGER_H_
#define LOGGER_H_

typedef enum {lvDebug, lvInfo, lvError} log_level_t;

#define LOG(_lvl, _msg) log_write(__FILE__, __LINE__, __func__, _lvl, _msg)
#define LOG1(_lvl, _msg, _p1) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1)
#define LOG2(_lvl, _msg, _p1, _p2) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2)
#define LOG3(_lvl, _msg, _p1, _p2, _p3) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2, _p3)
#define LOG4(_lvl, _msg, _p1, _p2, _p3, _p4) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2, _p3, _p4)
#define DEBUG(_msg) LOG(lvDebug, _msg)
#define DEBUG1(_msg, _p1) LOG1(lvDebug, _msg, _p1)
#define DEBUG2(_msg, _p1, _p2) LOG2(lvDebug, _msg, _p1, _p2)
#define DEBUG3(_msg, _p1, _p2, _p3) LOG3(lvDebug, _msg, _p1, _p2, _p3)
#define DEBUG4(_msg, _p1, _p2, _p3, _p4) LOG4(lvDebug, _msg, _p1, _p2, _p3, _p4)
#define INFO(_msg) LOG(lvInfo, _msg)
#define INFO1(_msg, _p1) LOG1(lvInfo, _msg, _p1)
#define INFO2(_msg, _p1, _p2) LOG2(lvInfo, _msg, _p1, _p2)
#define INFO3(_msg, _p1, _p2, _p3) LOG3(lvInfo, _msg, _p1, _p2, _p3)
#define INFO4(_msg, _p1, _p2, _p3, _p4) LOG4(lvInfo, _msg, _p1, _p2, _p3, _p4)
#define ERROR(_msg) LOG(lvError, _msg)
#define ERROR1(_msg, _p1) LOG1(lvError, _msg, _p1)
#define ERROR2(_msg, _p1, _p2) LOG2(lvError, _msg, _p1, _p2)
#define ERROR3(_msg, _p1, _p2, _p3) LOG3(lvError, _msg, _p1, _p2, _p3)
#define ERROR4(_msg, _p1, _p2, _p3, _p4) LOG4(lvError, _msg, _p1, _p2, _p3, _p4)

void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message,...);

void log_set_level(log_level_t level);

#endif /* LOGGER_H_ */
