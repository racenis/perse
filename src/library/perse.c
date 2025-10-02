#include "perse.h"

#include <stdarg.h>
#include <stdio.h>

void perse_DefaultLogger(const char* fmt, va_list args) {
    vprintf(fmt, args);
}

static void(*logger)(const char* fmt, ...) = perse_DefaultLogger;

/// Sets a custom logger.
/// Can be set to NULL, in which case no logging will occur.
void perse_SetLogger(void(*fn)(const char* fmt, ...)) {
	logger = fn;
}

/// Logs to logger.
/// The default logger will just forward parameters printf() and they will be
/// printed out to standard output. This behavior can be changed by setting some
/// other logging function by using perse_SetLogger().
void perse_Log(const char* fmt, ...) {
	if (!logger) return;
	
    va_list args;
    va_start(args, fmt);

	logger(fmt, args);
	
    va_end(args);
}
