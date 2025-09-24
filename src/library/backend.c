#include "backend.h"

#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// TODO: add unix .so loading code
// TODO: add emscripten bypass

void (*perse_BackendCreateWidget)(perse_widget_t*) = NULL;
void (*perse_BackendDestroyWidget)(perse_widget_t*) = NULL;

void (*perse_BackendSetProperty)(perse_widget_t*, perse_property_t*) = NULL;
void (*perse_BackendSetSizePos)(perse_widget_t*) = NULL;

void (*perse_BackendProcessEvents)() = NULL;

void (*perse_BackendSetLogger)(void(*)(const char* fmt, ...)) = NULL;

void perse_DefaultLogger(const char* fmt, ...);

void perse_LoadBackend() {
	HMODULE backend_lib = LoadLibrary("backend.dll");
	
	if (backend_lib == NULL) {
		printf("FATAL ERROR: FAILED TO LOAD BACKEND.DLL\n");
		abort();
	}
	
	// load widget functions
	perse_BackendCreateWidget =
		(void (*)(perse_widget_t*))GetProcAddress(backend_lib,
			"perse_impl_BackendCreateWidget");
	perse_BackendDestroyWidget =
		(void (*)(perse_widget_t*))GetProcAddress(backend_lib,
			"perse_impl_BackendDestroyWidget");
	perse_BackendSetProperty =
	   (void (*)(perse_widget_t*, perse_property_t*))GetProcAddress(backend_lib,
			"perse_impl_BackendSetProperty");
	perse_BackendSetSizePos =
		(void (*)(perse_widget_t*))GetProcAddress(backend_lib,
			"perse_impl_BackendSetSizePos");
	
	// load command functions
	perse_BackendProcessEvents =
		(void (*)())GetProcAddress(backend_lib,
			"perse_impl_BackendProcessEvents");
	
	// set up logging callback
	perse_BackendSetLogger =
		(void (*)(void(*)(const char* fmt, ...)))GetProcAddress(backend_lib,
			"perse_impl_SetLogger");
	perse_BackendSetLogger(perse_DefaultLogger);
}