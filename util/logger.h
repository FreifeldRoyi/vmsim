#ifndef LOGGER_H_
#define LOGGER_H_
/**A logger with several logging levels*/

//logging levels.
typedef enum {lvDebug, lvInfo, lvError} log_level_t;

/*The following macros allow printf-like usage of the logger.
 */

/**Log a message, optionally with some parameters.
 *
 * @param _lvl the log level of the message
 * @param _msg a printf format string to print.
 * @param _p1,_p2,_p3,_p4 optional parameters for the format string _msg
 * */
#define LOG(_lvl, _msg) log_write(__FILE__, __LINE__, __func__, _lvl, _msg)
#define LOG1(_lvl, _msg, _p1) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1)
#define LOG2(_lvl, _msg, _p1, _p2) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2)
#define LOG3(_lvl, _msg, _p1, _p2, _p3) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2, _p3)
#define LOG4(_lvl, _msg, _p1, _p2, _p3, _p4) log_write(__FILE__, __LINE__, __func__, _lvl, _msg, _p1, _p2, _p3, _p4)

/**Log an lvDebug-level message, optionally with some parameters.
 *
 * @param _msg a printf format string to print.
 * @param _p1,_p2,_p3,_p4 optional parameters for the format string _msg
 * */
#define DEBUG(_msg) LOG(lvDebug, _msg)
#define DEBUG1(_msg, _p1) LOG1(lvDebug, _msg, _p1)
#define DEBUG2(_msg, _p1, _p2) LOG2(lvDebug, _msg, _p1, _p2)
#define DEBUG3(_msg, _p1, _p2, _p3) LOG3(lvDebug, _msg, _p1, _p2, _p3)
#define DEBUG4(_msg, _p1, _p2, _p3, _p4) LOG4(lvDebug, _msg, _p1, _p2, _p3, _p4)

/**Log an lvInfo-level message, optionally with some parameters.
 *
 * @param _msg a printf format string to print.
 * @param _p1,_p2,_p3,_p4 optional parameters for the format string _msg
 * */
#define INFO(_msg) LOG(lvInfo, _msg)
#define INFO1(_msg, _p1) LOG1(lvInfo, _msg, _p1)
#define INFO2(_msg, _p1, _p2) LOG2(lvInfo, _msg, _p1, _p2)
#define INFO3(_msg, _p1, _p2, _p3) LOG3(lvInfo, _msg, _p1, _p2, _p3)
#define INFO4(_msg, _p1, _p2, _p3, _p4) LOG4(lvInfo, _msg, _p1, _p2, _p3, _p4)

/**Log an lvError-level message, optionally with some parameters.
 *
 * @param _msg a printf format string to print.
 * @param _p1,_p2,_p3,_p4 optional parameters for the format string _msg
 * */
#define ERROR(_msg) LOG(lvError, _msg)
#define ERROR1(_msg, _p1) LOG1(lvError, _msg, _p1)
#define ERROR2(_msg, _p1, _p2) LOG2(lvError, _msg, _p1, _p2)
#define ERROR3(_msg, _p1, _p2, _p3) LOG3(lvError, _msg, _p1, _p2, _p3)
#define ERROR4(_msg, _p1, _p2, _p3, _p4) LOG4(lvError, _msg, _p1, _p2, _p3, _p4)

/**Write to the log.
 * You shouldn't use this function. use one of the macros instead.
 * */
void log_write(	const char* file, int line, const char* func,
				log_level_t level, const char* message,...);

/**Set the current logging level.
 * @param level the new logging level. anything above or equal to this level will be printed.*/
void log_set_level(log_level_t level);

#endif /* LOGGER_H_ */
