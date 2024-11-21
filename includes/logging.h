#ifndef LOGGING_H
#define LOGGING_H

/// <summary>
/// Functions for logging useful colored messages to the console
/// </summary>
/// 
void log_info(const char* log, ...);
void log_info_no_prefix(const char* log, ...);
void log_debug(const char* log, ...);
void log_warning(const char* log, ...);
void log_error(const char* log, ...);

#endif