#include "backend.h"

#include "perse.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// TODO: add unix .so loading code
// TODO: add emscripten bypass

void (*perse_BackendCreateWidget)(perse_widget_t*) = NULL;
void (*perse_BackendDestroyWidget)(perse_widget_t*) = NULL;

void (*perse_BackendSetProperty)(perse_widget_t*, perse_property_t*) = NULL;
void (*perse_BackendSetSizePos)(perse_widget_t*) = NULL;

void (*perse_BackendProcessEvents)() = NULL;
int (*perse_BackendShouldQuit)() = NULL;

void (*perse_BackendSetLogger)(void(*)(const char* fmt, ...)) = NULL;

#define CHECK_FUNC(FUNC_NAME) \
	if (!FUNC_NAME) { \
		perse_Log(#FUNC_NAME " not loaded from BACKEND.DLL"); \
		abort(); \
	}

void perse_LoadBackend() {
	perse_Log("loading library\n");
	
	HMODULE backend_lib = LoadLibrary("backend.dll");
	
	if (backend_lib == NULL) {
		perse_Log("FATAL ERROR: FAILED TO LOAD BACKEND.DLL\n");
		abort();
	}
	
	perse_Log("getting funcs\n");
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
	perse_BackendShouldQuit =
		(int (*)())GetProcAddress(backend_lib,
			"perse_impl_BackendShouldQuit");
	
	// set up logging callback
	perse_BackendSetLogger =
		(void (*)(void(*)(const char* fmt, ...)))GetProcAddress(backend_lib,
			"perse_impl_BackendSetLogger");

	CHECK_FUNC(perse_BackendCreateWidget)
	CHECK_FUNC(perse_BackendDestroyWidget)
	CHECK_FUNC(perse_BackendSetProperty)
	CHECK_FUNC(perse_BackendSetSizePos)
	
	CHECK_FUNC(perse_BackendProcessEvents)
	CHECK_FUNC(perse_BackendShouldQuit)
	
	CHECK_FUNC(perse_BackendSetLogger)

	perse_BackendSetLogger(perse_Log);
}