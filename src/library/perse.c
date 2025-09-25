#include "perse.h"

#include <stdarg.h>
#include <stdio.h>

void perse_DefaultLogger(const char* fmt, va_list args) {
    vprintf(fmt, args);
}

static void(*logger)(const char* fmt, ...) = perse_DefaultLogger;

void perse_Log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

	logger(fmt, args);
	
    va_end(args);
}
